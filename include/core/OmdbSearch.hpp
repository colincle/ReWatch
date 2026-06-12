#pragma once

#include "Title.hpp"
#include "AppStorage.hpp"

#include <vector>
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>

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

    void search();
    void fetchById(const QString &imdbId, const QPixmap &posterImage);
    const results& getResults() const;

signals:
    void searchFinished();
    void titleFetched();

private slots:
    void onReplyFinished();

private:
    AppStorage &appStorage;

    results searchResults;

    QString baseUrl = "https://omdbapi.com/?apikey=";
    QString titleSearch = "&s=";
    QString idSearch = "&i=";
    QString requestUrl;
    QString apiKey;

    QNetworkAccessManager networkManager;

    int pendingPosters = 0;

    Title     titleFromJson(const QJsonObject &root, const QPixmap &posterImage);
    void      loadPosterForTitle(int i, const QString &posterUrl);
};