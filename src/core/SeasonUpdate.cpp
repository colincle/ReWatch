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

static constexpr int RECHECK_INTERVAL_DAYS = 14;

SeasonUpdate::SeasonUpdate(AppStorage &appStorage, QObject *parent)
	: QObject(parent)
	, appStorage(appStorage)
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
	if(!t.isSeries)
{
	return false;
}

return t.lastChecked.daysTo(QDate::currentDate()) > RECHECK_INTERVAL_DAYS;
}

namespace
{

struct SeasonFetchResult
{
	bool success = false;
	bool isAuthFailure = false;
	bool isNetworkFailure = false;
	QJsonObject data;
};

SeasonFetchResult fetchSeasonInfo(const QString &apiKey, const QString &imdbId)
{
	QNetworkAccessManager manager;
	QEventLoop loop;
	QNetworkReply *reply = manager.get(QNetworkRequest(OmdbSearch::makeUrl(apiKey, "i", imdbId)));
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	SeasonFetchResult result;

	if(reply->error() != QNetworkReply::NoError)
	{
		const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		const QString message = reply->errorString();
		reply->deleteLater();

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

	const QByteArray data = reply->readAll();
	reply->deleteLater();

	result.data = QJsonDocument::fromJson(data).object();

	if(result.data["Response"].toString() == "False")
	{
		result.isAuthFailure = OmdbSearch::isAuthError(result.data["Error"].toString());
		return result;
	}

	result.success = true;
	return result;
}

void applySeasonUpdate(Title &t, const QJsonObject &root)
{
	t.lastChecked = QDate::currentDate();

	const int newSeasons = root["totalSeasons"].toString().toInt();
	const int oldSeasons = t.totalSeasons.toInt();

	if(newSeasons > oldSeasons)
	{
		t.totalSeasons = QString::number(newSeasons);
		t.viewed = false;
	}
}

}

void SeasonUpdate::updateSeries()
{
	QMutexLocker locker(&appStorage.getMutex());

	for(Title *t : titles)
	{
		const SeasonFetchResult result = fetchSeasonInfo(appStorage.getKey(), t->imdbId);

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

		applySeasonUpdate(*t, result.data);
	}

	appStorage.save();
	emit seriesUpdated();
}
