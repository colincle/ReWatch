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
#include <QUrlQuery>
#include <utility>

QUrl OmdbSearch::makeUrl(
    const QString &apiKey, const QString &param, const QString &value
)
{
	QUrl      url("https://omdbapi.com/");
	QUrlQuery query;
	query.addQueryItem("apikey", apiKey);
	query.addQueryItem(param, value);
	url.setQuery(query);
	return url;
}

bool OmdbSearch::isAuthError(const QString &message)
{
	return message.contains("api key", Qt::CaseInsensitive) ||
	       message.contains("authentication", Qt::CaseInsensitive);
}

OmdbSearch::OmdbSearch(
    AppStorage &appStorage, QString query, QString key, QObject *parent
)
    : QObject(parent), appStorage(appStorage), apiKey(key)
{
	requestUrl = makeUrl(apiKey, "s", query).toString();
}

OmdbSearch::OmdbSearch(AppStorage &appStorage, QString key, QObject *parent)
    : QObject(parent), appStorage(appStorage), apiKey(key)
{
}

const Results &OmdbSearch::getResults() const
{
	return searchResults;
}

void OmdbSearch::search()
{
	QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(requestUrl)));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { onReplyFinished(reply); });
}

void OmdbSearch::fetchById(
    const QString &imdbId, const QPixmap &posterImage, bool posterNotFound
)
{
	QNetworkReply *reply =
	    networkManager.get(QNetworkRequest(makeUrl(apiKey, "i", imdbId)));

	connect(
	    reply,
	    &QNetworkReply::finished,
	    this,
	    [this, reply, posterImage, posterNotFound]()
	    { onFetchByIdFinished(reply, posterImage, posterNotFound); }
	);
}

void OmdbSearch::onFetchByIdFinished(
    QNetworkReply *reply, const QPixmap &posterImage, bool posterNotFound
)
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

	appStorage.addTitle(
	    titleFromOmdbJson(root, posterImage, posterNotFound),
	    posterImage
	);
	emit titleFetched();
}

Title OmdbSearch::titleFromOmdbJson(
    const QJsonObject &root, const QPixmap &posterImage, bool posterNotFound
)
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
	t.posterNotFound = posterNotFound;

	return t;
}

void OmdbSearch::loadDetailsForTitle(int i, const QString &imdbId)
{
	QNetworkReply *reply =
	    networkManager.get(QNetworkRequest(makeUrl(apiKey, "i", imdbId)));

	connect(
	    reply,
	    &QNetworkReply::finished,
	    this,
	    [this, i, reply]() { onDetailsFinished(reply, i); }
	);
}

void OmdbSearch::onDetailsFinished(QNetworkReply *reply, int i)
{
	QString posterUrl = "N/A";

	if(reply->error() == QNetworkReply::NoError)
	{
		QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
		searchResults.titles[i].plot = root["Plot"].toString();
		posterUrl = root["Poster"].toString();
	}

	reply->deleteLater();
	loadPosterForTitle(i, posterUrl);
}

void OmdbSearch::loadPosterForTitle(int i, const QString &posterUrl)
{
	if(posterUrl == "N/A")
	{
		searchResults.titles[i].posterImage.load(AssetsPaths::posterPlaceholder);
		searchResults.titles[i].posterNotFound = true;
		checkSearchComplete();
		return;
	}

	QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(posterUrl)));

	connect(
	    reply,
	    &QNetworkReply::finished,
	    this,
	    [this, i, reply]() { onPosterFinished(reply, i); }
	);
}

void OmdbSearch::onPosterFinished(QNetworkReply *reply, int i)
{
	QPixmap &image = searchResults.titles[i].posterImage;

	if(reply->error() != QNetworkReply::NoError || !image.loadFromData(reply->readAll()))
	{
		image.load(AssetsPaths::posterPlaceholder);
		searchResults.titles[i].posterNotFound = true;
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

static ResultTitle ResultTitleFromJson(const QJsonObject &obj)
{
	ResultTitle t;
	t.title = obj["Title"].toString();
	t.year = obj["Year"].toString();
	t.imdbId = obj["imdbID"].toString();
	t.type = obj["Type"].toString();
	t.poster = obj["Poster"].toString();
	return t;
}

static void classifyReplyError(QNetworkReply *reply, Results &out)
{
	const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	out.error = reply->errorString();
	out.errorType = (status == 401 || OmdbSearch::isAuthError(out.error))
	                    ? SearchErrorType::AuthInvalid
	                    : SearchErrorType::Network;
}

static void classifyResponseError(const QJsonObject &root, Results &out)
{
	out.error = root["Error"].toString();
	out.errorType = OmdbSearch::isAuthError(out.error) ? SearchErrorType::AuthInvalid
	                                                   : SearchErrorType::NotFound;
}

void OmdbSearch::onReplyFinished(QNetworkReply *reply)
{
	searchResults.error.clear();
	searchResults.errorType = SearchErrorType::None;
	searchResults.titles.clear();

	if(reply->error() != QNetworkReply::NoError)
	{
		classifyReplyError(reply, searchResults);
		reply->deleteLater();
		emit searchFinished();
		return;
	}

	QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
	reply->deleteLater();

	if(root["Response"].toString() == "False")
	{
		classifyResponseError(root, searchResults);
		emit searchFinished();
		return;
	}

	for(const QJsonValue &value : root["Search"].toArray())
	{
		ResultTitle t = ResultTitleFromJson(value.toObject());

		if(t.type == "movie" || t.type == "series")
		{
			searchResults.titles.push_back(std::move(t));
		}
	}

	pendingPosters = static_cast<int>(searchResults.titles.size());

	if(pendingPosters == 0)
	{
		emit searchFinished();
		return;
	}

	for(int i = 0; i < static_cast<int>(searchResults.titles.size()); ++i)
	{
		loadDetailsForTitle(i, searchResults.titles[i].imdbId);
	}
}