// OMDb API client. Handles both text searches (search/searchFinished) and single-title
// fetches by IMDb ID (fetchById/titleFetched). Two-phase: fetches details then posters.
#pragma once

#include "AppStorage.hpp"
#include "Title.hpp"

#include <vector>
#include <QDate>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QUrl>

struct ResultTitle
{
	QString title;
	QString plot;
	QString imdbId;
	QString type;
	QString poster;
	QPixmap posterImage;
	bool    posterNotFound = false;
};

enum class SearchErrorType
{
	None,
	Network,
	AuthInvalid,
	NotFound,
	RateLimited
};

struct Results
{
	SearchErrorType          errorType = SearchErrorType::None;
	QString                  error;
	std::vector<ResultTitle> titles;
};

class OmdbSearch : public QObject
{
	Q_OBJECT

  public:
	OmdbSearch(
	    AppStorage &appStorage, QString query, QString apiKey, QObject *parent = nullptr
	);
	OmdbSearch(AppStorage &appStorage, QString apiKey, QObject *parent = nullptr);

	void search();
	void
	fetchById(const QString &imdbId, const QPixmap &posterImage, bool posterNotFound);
	const Results &getResults() const;

	static QUrl
	makeUrl(const QString &apiKey, const QString &param, const QString &value);
	static bool isAuthError(const QString &message);
	static bool isRateLimitError(const QString &message);
	static bool fetchSeasonJson(
	    QNetworkAccessManager &manager, const QString &apiKey, const QString &imdbId,
	    int season, QJsonObject &out
	);
	static SearchErrorType findLastEpisode(
	    QNetworkAccessManager &manager, const QString &apiKey, const QString &imdbId,
	    LastEpisode &le, QString &nextSeasonDate, const QDate &showReleaseDate,
	    int *requestsMade = nullptr
	);

  signals:
	void searchFinished();
	void titleFetched();
	void titleFetchFailed();
	void rateLimitReached();

  private:
	AppStorage &appStorage;

	Results searchResults;

	QString requestUrl;
	QString apiKey;

	QNetworkAccessManager networkManager;

	int pendingPosters = 0;

	Title titleFromOmdbJson(
	    const QJsonObject &root, const QPixmap &posterImage, bool posterNotFound
	);
	void loadDetailsForTitle(int i, const QString &imdbId);
	void onDetailsFinished(QNetworkReply *reply, int i);
	void loadPosterForTitle(int i, const QString &posterUrl);
	void onFetchByIdFinished(
	    QNetworkReply *reply, const QPixmap &posterImage, bool posterNotFound
	);
	void onPosterFinished(QNetworkReply *reply, int i);
	void onReplyFinished(QNetworkReply *reply);
	void checkSearchComplete();
};
