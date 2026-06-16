#include "OmdbSearch.hpp"
#include "AssetsPaths.hpp"
#include "Title.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QUrl>

static const QString BASE_URL = "https://omdbapi.com/?apikey=";
static const QString TITLE_SEARCH = "&s=";
static const QString ID_SEARCH = "&i=";

OmdbSearch::OmdbSearch(AppStorage &appStorage, QString query, QString key, QObject *parent)
	: QObject(parent)
	, appStorage(appStorage)
	, apiKey(key)
{
	query.replace(' ', '+');
	requestUrl = BASE_URL + apiKey + TITLE_SEARCH + query;
}

OmdbSearch::OmdbSearch(AppStorage &appStorage, QString key, QObject *parent)
	: QObject(parent)
	, appStorage(appStorage)
	, apiKey(key)
{
	requestUrl = BASE_URL + apiKey + TITLE_SEARCH;
}

const results &OmdbSearch::getResults() const
{
	return searchResults;
}

void OmdbSearch::search()
{
	QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(requestUrl)));
	connect(reply, &QNetworkReply::finished, this, &OmdbSearch::onReplyFinished);
}

void OmdbSearch::fetchById(const QString &imdbId, const QPixmap &posterImage)
{
	QString url = BASE_URL + apiKey + ID_SEARCH + imdbId;
	QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(url)));

	connect(reply, &QNetworkReply::finished, this, [this, reply, posterImage]()
	{
		onFetchByIdFinished(reply, posterImage);
	});
}

void OmdbSearch::onFetchByIdFinished(QNetworkReply *reply, const QPixmap &posterImage)
{
	if(reply->error() != QNetworkReply::NoError)
	{
		reply->deleteLater();
		emit titleFetchFailed();
		return;
	}

	QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
	reply->deleteLater();

	if(root["Response"].toString() == "False")
	{
		emit titleFetchFailed();
		return;
	}

	appStorage.addTitle(titleFromJson(root, posterImage), posterImage);
	emit titleFetched();
}

Title OmdbSearch::titleFromJson(const QJsonObject &root, const QPixmap &posterImage)
{
	Title t;

	t.title = root["Title"].toString();
	t.year = root["Year"].toString();
	t.imdbId = root["imdbID"].toString();
	t.type = root["Type"].toString();
	t.released = root["Released"].toString();
	t.plot = root["Plot"].toString();
	t.director = root["Director"].toString();
	t.actors = root["Actors"].toString();
	t.totalSeasons = root["totalSeasons"].toString();
	t.posterImage = posterImage;
	t.isMovie = t.type == "movie";
	t.isSeries = t.type == "series";

	return t;
}

void OmdbSearch::loadPosterForTitle(int i, const QString &posterUrl)
{
	if(posterUrl == "N/A")
	{
		searchResults.titles[i].posterImage.load(POSTER_PLACEHOLDER);
		checkSearchComplete();
		return;
	}

	QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(posterUrl)));

	connect(reply, &QNetworkReply::finished, this, [this, i, reply]()
	{
		onPosterFinished(reply, i);
	});
}

void OmdbSearch::onPosterFinished(QNetworkReply *reply, int i)
{
	QPixmap &image = searchResults.titles[i].posterImage;

	if(reply->error() != QNetworkReply::NoError || !image.loadFromData(reply->readAll()))
	{
		image.load(POSTER_PLACEHOLDER);
	}

	reply->deleteLater();
	checkSearchComplete();
}

void OmdbSearch::checkSearchComplete()
{
	if(--pendingPosters == 0)
	{
		emit searchFinished();
	}
}

static resultTitle resultTitleFromJson(const QJsonObject &obj)
{
	resultTitle t;
	t.title = obj["Title"].toString();
	t.year = obj["Year"].toString();
	t.imdbId = obj["imdbID"].toString();
	t.type = obj["Type"].toString();
	t.poster = obj["Poster"].toString();
	return t;
}

void OmdbSearch::onReplyFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

	searchResults.error.clear();
	searchResults.titles.clear();

	if(!reply)
	{
		searchResults.error = "Internal error";
		emit searchFinished();
		return;
	}

	if(reply->error() != QNetworkReply::NoError)
	{
		searchResults.error = reply->errorString();
		reply->deleteLater();
		emit searchFinished();
		return;
	}

	QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
	reply->deleteLater();

	if(root["Response"].toString() == "False")
	{
		searchResults.error = root["Error"].toString();
		emit searchFinished();
		return;
	}

	for(const QJsonValue &value : root["Search"].toArray())
	{
		searchResults.titles.push_back(resultTitleFromJson(value.toObject()));
	}

	pendingPosters = static_cast<int>(searchResults.titles.size());

	if(pendingPosters == 0)
	{
		emit searchFinished();
		return;
	}

	for(int i = 0; i < static_cast<int>(searchResults.titles.size()); ++i)
	{
		loadPosterForTitle(i, searchResults.titles[i].poster);
	}
}