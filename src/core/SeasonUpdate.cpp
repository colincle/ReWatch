#include "SeasonUpdate.hpp"
#include "OmdbSearch.hpp"

#include <QDate>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

SeasonUpdate::SeasonUpdate(AppStorage &appStorage, QObject *parent)
    : QObject(parent), appStorage(appStorage)
{
	auto lock = appStorage.lock();

	for(const Title &t : appStorage.getTitlesMutable(lock))
		if(isEligible(t))
			imdbIds.push_back(t.imdbId);

	if(imdbIds.empty())
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

	if(!t.lastChecked.isValid())
		return true;

	return t.lastChecked != QDate::currentDate();
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
	auto lock = appStorage.lock();

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

		auto &titles = appStorage.getTitlesMutable(lock);
		auto  it = std::find_if(
		    titles.begin(),
		    titles.end(),
		    [&](const Title &t) { return t.imdbId == imdbIds[i]; }
		);

		if(it != titles.end())
			applySeasonUpdate(*it, result.data, notifications);
	}

	appStorage.addNotifications(notifications);
	emit seriesUpdated();
}
