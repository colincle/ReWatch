// Two-phase search: first fetches details (plot, poster URL) for each result, then
// downloads poster images. Poster failures fall back to the placeholder silently.
#include "OmdbSearch.hpp"
#include "AssetsPaths.hpp"
#include "Title.hpp"

#include <QDate>
#include <QEventLoop>
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

bool OmdbSearch::isRateLimitError(const QString &message)
{
	return message.contains("request limit", Qt::CaseInsensitive);
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
	connect(
	    reply,
	    &QNetworkReply::finished,
	    this,
	    [this, reply]() { onReplyFinished(reply); }
	);
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
		if(isRateLimitError(root["Error"].toString()))
			emit rateLimitReached();
		emit titleFetchFailed();
		return;
	}

	Title t = titleFromOmdbJson(root, posterImage, posterNotFound);

	if(t.type == "series")
	{
		const QDate           showRelease = QDate::fromString(t.released, "dd MMM yyyy");
		const SearchErrorType err = OmdbSearch::findLastEpisode(
		    networkManager,
		    apiKey,
		    t.imdbId,
		    t.lastEpisode,
		    t.nextSeasonDate,
		    showRelease
		);
		if(err == SearchErrorType::RateLimited)
		{
			emit rateLimitReached();
			emit titleFetchFailed();
			return;
		}
		if(err != SearchErrorType::None)
		{
			emit titleFetchFailed();
			return;
		}
	}

	appStorage.addTitle(t, posterImage);
	emit titleFetched();
}

Title OmdbSearch::titleFromOmdbJson(
    const QJsonObject &root, const QPixmap &posterImage, bool posterNotFound
)
{
	Title t;

	t.title = root["Title"].toString();
	t.imdbId = root["imdbID"].toString();
	t.type = root["Type"].toString();
	t.released = root["Released"].toString();
	t.plot = root["Plot"].toString();
	t.director = root["Director"].toString();
	t.actors = root["Actors"].toString();
	t.lastEpisode.season = root["totalSeasons"].toString().toInt();
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

		if(root["Response"].toString() == "False")
		{
			if(isRateLimitError(root["Error"].toString()))
				emit rateLimitReached();
		}
		else
		{
			searchResults.titles[i].plot = root["Plot"].toString();
			posterUrl = root["Poster"].toString();
		}
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
	if(OmdbSearch::isAuthError(out.error))
		out.errorType = SearchErrorType::AuthInvalid;
	else if(OmdbSearch::isRateLimitError(out.error))
		out.errorType = SearchErrorType::RateLimited;
	else
		out.errorType = SearchErrorType::NotFound;
}

void OmdbSearch::onReplyFinished(QNetworkReply *reply)
{
	searchResults.error.clear();
	searchResults.errorType = SearchErrorType::None;
	searchResults.titles.clear();

	const QByteArray  raw = reply->readAll();
	const bool        err = reply->error() != QNetworkReply::NoError;
	const QJsonObject root = QJsonDocument::fromJson(raw).object();

	if(root["Response"].toString() == "False")
	{
		classifyResponseError(root, searchResults);
		reply->deleteLater();
		emit searchFinished();
		return;
	}

	if(err)
	{
		classifyReplyError(reply, searchResults);
		reply->deleteLater();
		emit searchFinished();
		return;
	}

	reply->deleteLater();

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

bool OmdbSearch::fetchSeasonJson(
    QNetworkAccessManager &manager, const QString &apiKey, const QString &imdbId,
    int season, QJsonObject &out
)
{
	QUrl      url("https://omdbapi.com/");
	QUrlQuery query;
	query.addQueryItem("apikey", apiKey);
	query.addQueryItem("i", imdbId);
	query.addQueryItem("Season", QString::number(season));
	url.setQuery(query);

	QNetworkReply *reply = manager.get(QNetworkRequest(url));
	QEventLoop     loop;
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	const bool ok = reply->error() == QNetworkReply::NoError;
	out = QJsonDocument::fromJson(reply->readAll()).object();
	reply->deleteLater();
	return ok;
}

// Dates earlier than the show's own release are OMDb placeholders; treated as not yet
// aired.
static bool isSeasonNotYetAired(
    const QString &firstReleasedStr, const QDate &showReleaseDate, QString &nextSeasonDate
)
{
	const QDate firstAired = QDate::fromString(firstReleasedStr, "yyyy-MM-dd");
	const bool  isBogus = showReleaseDate.isValid() && firstAired < showReleaseDate;

	if(!firstAired.isValid() || isBogus || firstAired > QDate::currentDate())
	{
		if(firstAired.isValid() && !isBogus && firstAired > QDate::currentDate())
			nextSeasonDate = firstReleasedStr;
		return true;
	}
	return false;
}

static int
lastAiredEpisodeNumber(const QJsonArray &episodes, const QDate &showReleaseDate)
{
	for(int i = episodes.size() - 1; i >= 0; --i)
	{
		const QDate epDate = QDate::fromString(
		    episodes[i].toObject()["Released"].toString(),
		    "yyyy-MM-dd"
		);
		const bool epIsBogus = showReleaseDate.isValid() && epDate < showReleaseDate;
		if(epDate.isValid() && !epIsBogus && epDate <= QDate::currentDate())
			return episodes[i].toObject()["Episode"].toString().toInt();
	}
	return 1;
}

SearchErrorType OmdbSearch::findLastEpisode(
    QNetworkAccessManager &manager, const QString &apiKey, const QString &imdbId,
    LastEpisode &le, QString &nextSeasonDate, const QDate &showReleaseDate,
    int *requestsMade
)
{
	nextSeasonDate.clear();

	while(le.season >= 1)
	{
		QJsonObject data;
		const bool  ok = fetchSeasonJson(manager, apiKey, imdbId, le.season, data);
		if(requestsMade)
			(*requestsMade)++;
		if(!ok)
			return SearchErrorType::Network;

		if(data["Response"].toString() == "False")
		{
			const QString error = data["Error"].toString();
			if(isAuthError(error))
				return SearchErrorType::AuthInvalid;
			if(isRateLimitError(error))
				return SearchErrorType::RateLimited;
			le.season--;
			continue;
		}

		const QJsonArray episodes = data["Episodes"].toArray();
		if(episodes.isEmpty())
		{
			le.season--;
			continue;
		}

		const QString firstReleasedStr = episodes[0].toObject()["Released"].toString();
		if(isSeasonNotYetAired(firstReleasedStr, showReleaseDate, nextSeasonDate))
		{
			le.season--;
			continue;
		}

		le.episode = lastAiredEpisodeNumber(episodes, showReleaseDate);
		return SearchErrorType::None;
	}

	le = {1, 1};
	return SearchErrorType::None;
}