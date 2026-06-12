#include "OmdbSearch.hpp"
#include "AssetsPaths.hpp"
#include "Title.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QPixmap>

OmdbSearch::OmdbSearch(AppStorage &appStorage,
                        QString query,
                        QString key,
                        QObject *parent)
    : appStorage(appStorage), apiKey(key), QObject(parent)
{
    query.replace(' ', '+');
    requestUrl = baseUrl + apiKey + titleSearch + query;
}

const results& OmdbSearch::getResults() const
{
    return searchResults;
}

void OmdbSearch::search()
{
    QNetworkReply *reply =
        networkManager.get(QNetworkRequest(QUrl(requestUrl)));

    connect(reply,
            &QNetworkReply::finished,
            this,
            &OmdbSearch::onReplyFinished);
}

void OmdbSearch::fetchById(const QString &imdbId, const QPixmap &posterImage)
{
    QString url = baseUrl + apiKey + idSearch + imdbId;

    QNetworkReply *reply =
        networkManager.get(QNetworkRequest(QUrl(url)));

    connect(reply, &QNetworkReply::finished, this,
        [this, reply, posterImage]()
        {
            if (reply->error() != QNetworkReply::NoError)
            {
                reply->deleteLater();
                return;
            }

            QJsonObject root =
                QJsonDocument::fromJson(reply->readAll()).object();

            reply->deleteLater();

            if (root["Response"].toString() == "False")
                return;

            appStorage.addTitle(titleFromJson(root, posterImage), posterImage);
            emit titleFetched();
        }
    );
}


Title OmdbSearch::titleFromJson(const QJsonObject &root, const QPixmap &posterImage)
{
    Title t;

    t.title        = root["Title"].toString();
    t.year         = root["Year"].toString();
    t.imdbId       = root["imdbID"].toString();
    t.type         = root["Type"].toString();
    t.released     = root["Released"].toString();
    t.plot         = root["Plot"].toString();
    t.director     = root["Director"].toString();
    t.actors       = root["Actors"].toString();
    t.totalSeasons = root["totalSeasons"].toString();
    t.posterImage  = posterImage;
    t.isMovie      = t.type == "movie";
    t.isSeries     = t.type == "series";

    return t;
}

void OmdbSearch::loadPosterForTitle(int i, const QString &posterUrl)
{
    if (posterUrl == "N/A")
    {
        searchResults.titles[i].posterImage.load(POSTER_PLACEHOLDER);

        if (--pendingPosters == 0)
            emit searchFinished();

        return;
    }

    QNetworkReply *posterReply =
        networkManager.get(
            QNetworkRequest(QUrl(posterUrl)));

    connect(posterReply, &QNetworkReply::finished, this,
        [this, i, posterReply]()
        {
            QPixmap &image = searchResults.titles[i].posterImage;

            if (posterReply->error() != QNetworkReply::NoError ||
                !image.loadFromData(posterReply->readAll()))
            {
                image.load(POSTER_PLACEHOLDER);
            }

            posterReply->deleteLater();

            if (--pendingPosters == 0)
                emit searchFinished();
        }
    );
}

void OmdbSearch::onReplyFinished()
{
    QNetworkReply *reply =
        qobject_cast<QNetworkReply*>(sender());

    searchResults.error.clear();
    searchResults.titles.clear();

    if (!reply)
    {
        searchResults.error = "Internal error";
        emit searchFinished();
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        searchResults.error = reply->errorString();
        reply->deleteLater();
        emit searchFinished();
        return;
    }

    QJsonObject root =
        QJsonDocument::fromJson(reply->readAll()).object();

    reply->deleteLater();

    if (root["Response"].toString() == "False")
    {
        searchResults.error = root["Error"].toString();
        emit searchFinished();
        return;
    }

    for (const QJsonValue &value : root["Search"].toArray())
    {
        QJsonObject obj = value.toObject();

        resultTitle title;
        title.title  = obj["Title"].toString();
        title.year   = obj["Year"].toString();
        title.imdbId = obj["imdbID"].toString();
        title.type   = obj["Type"].toString();
        title.poster = obj["Poster"].toString();

        searchResults.titles.push_back(title);
    }

    pendingPosters = static_cast<int>(searchResults.titles.size());

    if (pendingPosters == 0)
    {
        emit searchFinished();
        return;
    }

    for (int i = 0; i < static_cast<int>(searchResults.titles.size()); ++i)
        loadPosterForTitle(i, searchResults.titles[i].poster);
}