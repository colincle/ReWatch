#include "SeasonUpdate.hpp"
#include "OmdbSearch.hpp"

#include <QDate>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

static constexpr int RECHECK_INTERVAL_DAYS = 1;

SeasonUpdate::SeasonUpdate(AppStorage &appStorage, QObject *parent)
    : QObject(parent), appStorage(appStorage)
{
	QMutexLocker locker(&appStorage.getMutex());

	for(Title &t : appStorage.getTitlesMutable())
		if(isEligible(t))
		{
			titles.push_back(&t);
		}

	if(titles.empty())
	{
		queueEmpty = true;
	}
}

bool SeasonUpdate::isEligible(const Title &t) const
{
	if(t.type != "series")
	{
		return false;
	}

	return t.lastChecked.daysTo(QDate::currentDate()) > RECHECK_INTERVAL_DAYS;
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
	QMutexLocker locker(&appStorage.getMutex());

	const int                  count = static_cast<int>(titles.size());
	QNetworkAccessManager      manager;
	QVector<SeasonFetchResult> results(count);
	int                        pending = count;
	QEventLoop                 loop;

	for(int i = 0; i < count; ++i)
	{
		QNetworkReply *reply = manager.get(QNetworkRequest(
		    OmdbSearch::makeUrl(appStorage.getKey(), "i", titles[i]->imdbId)
		));

		QObject::connect(
		    reply,
		    &QNetworkReply::finished,
		    &loop,
		    [&, i, reply]()
		    {
			    results[i] = parseReply(reply);
			    reply->deleteLater();

			    if(--pending == 0)
			    {
				    loop.quit();
			    }
		    }
		);
	}

	if(pending > 0)
	{
		loop.exec();
	}

	std::vector<QString> notifications;

	for(int i = 0; i < count; ++i)
	{
		const SeasonFetchResult &result = results[i];

		if(result.isAuthFailure)
		{
			emit apiKeyError();
			return;
		}

		if(result.isNetworkFailure)
		{
			emit networkError();
			return;
		}

		if(!result.success)
		{
			continue;
		}

		applySeasonUpdate(*titles[i], result.data, notifications);
	}

	appStorage.addNotifications(notifications);
	emit seriesUpdated();
}
