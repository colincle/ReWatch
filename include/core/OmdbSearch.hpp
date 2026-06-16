#pragma once

#include "AppStorage.hpp"
#include "Title.hpp"

#include <vector>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPixmap>
#include <QString>

struct resultTitle
{
	QString title;
	QString year;
	QString imdbId;
	QString type;
	QString poster;
	QPixmap posterImage;
};

struct results
{
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
	void fetchById(const QString &imdbId, const QPixmap &posterImage);
	const results &getResults() const;

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

	Title titleFromJson(const QJsonObject &root, const QPixmap &posterImage);
	void loadPosterForTitle(int i, const QString &posterUrl);
	void onFetchByIdFinished(QNetworkReply *reply, const QPixmap &posterImage);
	void onPosterFinished(QNetworkReply *reply, int i);
	void checkSearchComplete();
};
