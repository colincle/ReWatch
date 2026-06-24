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

	// Collect all eligible IDs and build a priority-ordered list.
	// IDs that were skipped last run (updatePriority) go first so they
	// eventually get a turn even when the library exceeds the request limit.
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

	// Record requests consumed today and persist skipped IDs for next run.
	appStorage.addUpdateChecks(take);
	std::vector<QString> overflow(ordered.begin() + take, ordered.end());
	appStorage.setUpdatePriority(std::move(overflow));

	if(imdbIds.isEmpty())
		queueEmpty = true;
}

static bool isFinished(const Title &t)
{
	// OMDb returns "YYYY–" for ongoing and "YYYY–YYYY" for ended series.
	// A series is finished when there are digits after the last dash/en-dash.
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
	// Network phase — no lock needed, imdbIds were captured in the constructor
	// and getKey() acquires the mutex internally.
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

	// Mutation phase — acquire lock only here, for the minimum duration.
	// Process all results: apply successful ones, track errors without bailing
	// early so that shows with valid responses get their lastChecked updated.
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
