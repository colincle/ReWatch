// Constructs the priority-ordered, budget-capped update queue and runs all OMDb fetches
// synchronously in updateSeries(), which is designed to run on a background thread.
#include "SeasonUpdate.hpp"
#include "OmdbSearch.hpp"

#include <QDate>
#include <QEventLoop>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

SeasonUpdate::SeasonUpdate(AppStorage &appStorage, QObject *parent)
    : QObject(parent), appStorage(appStorage)
{
	auto lock = appStorage.lock();

	// Priority IDs (skipped last run) go first so they eventually get a turn.
	const auto &priority = appStorage.getUpdatePriority(lock);
	const QDate today = QDate::currentDate();
	const int   usedToday =
	    (appStorage.getChecksDate() == today) ? appStorage.getChecksToday() : 0;
	const int maxRequests = std::max(0, appStorage.getMaxUpdateRequests() - usedToday);

	QSet<QString> prioritySet(priority.begin(), priority.end());
	QSet<QString> eligibleSet;

	for(const Title &t : appStorage.getTitlesMutable(lock))
		if(isEligible(t))
			eligibleSet.insert(t.imdbId);

	QVector<QString> ordered;
	for(const QString &id : priority)
		if(eligibleSet.contains(id))
			ordered.push_back(id);
	for(const Title &t : appStorage.getTitlesMutable(lock))
		if(isEligible(t) && !prioritySet.contains(t.imdbId))
			ordered.push_back(t.imdbId);

	const int take = std::min(static_cast<int>(ordered.size()), maxRequests);
	imdbIds = ordered.mid(0, take);

	appStorage.addUpdateChecks(take);
	std::vector<QString> overflow(ordered.begin() + take, ordered.end());
	appStorage.setUpdatePriority(std::move(overflow));

	if(imdbIds.isEmpty())
		queueEmpty = true;
}

static bool isFinished(const Title &t)
{
	// OMDb uses "YYYY–" for ongoing, "YYYY–YYYY" for ended. Finished = digits after the
	// last separator. Both plain hyphen and en-dash (0x2013) appear in the wild.
	for(QChar sep : {QChar('-'), QChar(0x2013)})
	{
		const int pos = t.year.lastIndexOf(sep);
		if(pos != -1 && pos < t.year.size() - 1)
			return true;
	}
	return false;
}

bool SeasonUpdate::isEligible(const Title &t) const
{
	if(t.type != "series")
		return false;

	if(!t.lastChecked.isValid())
		return true;

	const int intervalDays = isFinished(t) ? 30 : 1;
	return t.lastChecked.daysTo(QDate::currentDate()) >= intervalDays;
}

namespace
{

struct SeasonFetchResult
{
	bool        success = false;
	bool        isAuthFailure = false;
	bool        isNetworkFailure = false;
	QJsonObject data;
};

SeasonFetchResult parseReply(QNetworkReply *reply)
{
	SeasonFetchResult result;

	if(reply->error() != QNetworkReply::NoError)
	{
		const int status =
		    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		const QString message = reply->errorString();

		if(status == 401 || OmdbSearch::isAuthError(message))
		{
			result.isAuthFailure = true;
		}
		else
		{
			result.isNetworkFailure = true;
		}

		return result;
	}

	result.data = QJsonDocument::fromJson(reply->readAll()).object();

	if(result.data["Response"].toString() == "False")
	{
		result.isAuthFailure = OmdbSearch::isAuthError(result.data["Error"].toString());
		return result;
	}

	result.success = true;
	return result;
}

void applySeasonUpdate(
    Title &t, const QJsonObject &root, std::vector<QString> &notifications
)
{
	t.lastChecked = QDate::currentDate();

	const int newSeasons = root["totalSeasons"].toString().toInt();
	const int oldSeasons = t.totalSeasons.toInt();

	if(newSeasons > oldSeasons)
	{
		t.totalSeasons = QString::number(newSeasons);
		t.viewed = false;
		notifications.push_back(t.imdbId);
	}
}

} // namespace

void SeasonUpdate::updateSeries()
{
	// Network phase — no lock held; imdbIds captured at construction, getKey() locks internally.
	const int                  count = static_cast<int>(imdbIds.size());
	QNetworkAccessManager      manager;
	QVector<SeasonFetchResult> results(count);
	int                        pending = count;
	QEventLoop                 loop;

	for(int i = 0; i < count; ++i)
	{
		QNetworkReply *reply = manager.get(
		    QNetworkRequest(OmdbSearch::makeUrl(appStorage.getKey(), "i", imdbIds[i]))
		);

		QObject::connect(
		    reply,
		    &QNetworkReply::finished,
		    &loop,
		    [&, i, reply]()
		    {
			    results[i] = parseReply(reply);
			    reply->deleteLater();

			    if(--pending == 0)
				    loop.quit();
		    }
		);
	}

	if(pending > 0)
		loop.exec();

	// Lock only for the mutation phase. Apply all successful results even when others
	// failed — partial progress is better than rolling back the whole batch.
	std::vector<QString> notifications;
	bool                 hasAuthError = false;
	bool                 hasNetworkError = false;
	{
		auto  lock = appStorage.lock();
		auto &titles = appStorage.getTitlesMutable(lock);

		for(int i = 0; i < count; ++i)
		{
			if(results[i].isAuthFailure)
			{
				hasAuthError = true;
				continue;
			}
			if(results[i].isNetworkFailure)
			{
				hasNetworkError = true;
				continue;
			}
			if(!results[i].success)
				continue;

			auto it = std::find_if(
			    titles.begin(),
			    titles.end(),
			    [&](const Title &t) { return t.imdbId == imdbIds[i]; }
			);

			if(it != titles.end())
				applySeasonUpdate(*it, results[i].data, notifications);
		}
	}

	appStorage.addNotifications(notifications);
	emit seriesUpdated();

	if(hasAuthError)
		emit apiKeyError();
	else if(hasNetworkError)
		emit networkError();
}
