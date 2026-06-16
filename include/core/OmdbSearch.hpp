#pragma once

#include "AppStorage.hpp"
#include "Title.hpp"

#include <vector>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QUrl>

struct resultTitle
{
	QString title;
	QString year;
	QString plot;
	QString imdbId;
	QString type;
	QString poster;
	QPixmap posterImage;
	bool posterNotFound = false;
};

enum class SearchErrorType
{
	None,
	Network,
	AuthInvalid,
	NotFound
};

struct results
{
	SearchErrorType errorType = SearchErrorType::None;
	QString error;
	std::vector<resultTitle> titles;
};

class OmdbSearch : public QObject
{
	Q_OBJECT

public:
	OmdbSearch(AppStorage &appStorage, QString query, QString apiKey, QObject *parent = nullptr);
	OmdbSearch(AppStorage &appStorage, QString apiKey, QObject *parent = nullptr);

	void search();
	void fetchById(const QString &imdbId, const QPixmap &posterImage, bool posterNotFound);
	const results &getResults() const;

	static QUrl makeUrl(const QString &apiKey, const QString &param, const QString &value);
	static bool isAuthError(const QString &message);

signals:
	void searchFinished();
	void titleFetched();
	void titleFetchFailed();

private slots:
	void onReplyFinished();

private:
	AppStorage &appStorage;

	results searchResults;

	QString requestUrl;
	QString apiKey;

	QNetworkAccessManager networkManager;

	int pendingPosters = 0;

	Title titleFromOmdbJson(const QJsonObject &root, const QPixmap &posterImage, bool posterNotFound);
	void loadDetailsForTitle(int i, const QString &imdbId);
	void onDetailsFinished(QNetworkReply *reply, int i);
	void loadPosterForTitle(int i, const QString &posterUrl);
	void onFetchByIdFinished(QNetworkReply *reply, const QPixmap &posterImage, bool posterNotFound);
	void onPosterFinished(QNetworkReply *reply, int i);
	void checkSearchComplete();
};
