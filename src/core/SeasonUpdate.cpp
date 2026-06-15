#include "SeasonUpdate.hpp"

#include <QDate>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

static constexpr int RECHECK_INTERVAL_DAYS = 14;

SeasonUpdate::SeasonUpdate(AppStorage &appStorage, QObject *parent)
	: QObject(parent)
	, appStorage(appStorage)
{
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

void SeasonUpdate::updateSeries()
{
	QNetworkAccessManager manager;

	for(Title *t : titles)
	{
		QString url = "https://omdbapi.com/?apikey=" + appStorage.getKey() + "&i=" + t->imdbId;

		QEventLoop loop;
		QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
		QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
		loop.exec();

		if(reply->error() != QNetworkReply::NoError)
		{
			reply->deleteLater();
			emit apiKeyError();
			return;
		}

		QByteArray data = reply->readAll();
		reply->deleteLater();

		QJsonObject root = QJsonDocument::fromJson(data).object();

		if(root["Response"].toString() == "False")
		{
			continue;
		}

		t->lastChecked = QDate::currentDate();

		int newSeasons = root["totalSeasons"].toString().toInt();
		int oldSeasons = t->totalSeasons.toInt();

		if(newSeasons > oldSeasons)
		{
			t->totalSeasons = QString::number(newSeasons);
			t->viewed = false;
		}
	}

	appStorage.save();
}