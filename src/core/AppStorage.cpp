#include "AppStorage.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AppStorage::AppStorage()
{
    QString dirPath = QDir::homePath() + "/.local/share/movieTracker";
    QString filePath = dirPath + "/movieTracker.json";
    postersPath = dirPath + "/Posters";

    appFilePath = filePath;

    QDir dir(dirPath);

    if (!dir.exists())
        dir.mkpath(dirPath);

    QDir postersDir(postersPath);

    if (!postersDir.exists())
        postersDir.mkpath(postersPath);

    QFile file(filePath);

    if (!file.exists())
    {
        if (file.open(QIODevice::WriteOnly))
        {
            file.write("{\n\t\"omdbApiKey\": \"\",\n\t\"titles\": []\n}\n");
            file.close();
        }
    }

    load();
}

void AppStorage::load()
{
    QFile file(appFilePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file for reading:" << appFilePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    omdbApiKey = root["omdbApiKey"].toString();

    titles.clear();

    QJsonArray arr = root["titles"].toArray();

    for (const QJsonValue &val : arr)
    {
        QJsonObject obj = val.toObject();

        Title t;

        t.title        = obj["title"].toString();
        t.year         = obj["year"].toString();
        t.imdbId       = obj["imdbId"].toString();
        t.type         = obj["type"].toString();

        t.released     = obj["released"].toString();
        t.plot         = obj["plot"].toString();

        t.director     = obj["director"].toString();
        t.actors       = obj["actors"].toString();

        t.totalSeasons = obj["totalSeasons"].toString();

        t.isMovie      = t.type == "movie";
        t.isSeries     = t.type == "series";

        QString posterFile =
            postersPath + "/" + t.imdbId + ".png";

        if (!t.posterImage.load(posterFile))
        {
            qWarning() << "Failed to load poster:" << posterFile;
        }

        titles.push_back(std::move(t));
    }
}

void AppStorage::save()
{
    QJsonObject root;

    root["omdbApiKey"] = omdbApiKey;

    QJsonArray arr;

    for (const Title &t : titles)
    {
        QJsonObject obj;

        obj["title"]        = t.title;
        obj["year"]         = t.year;
        obj["imdbId"]       = t.imdbId;
        obj["type"]         = t.type;

        obj["released"]     = t.released;
        obj["plot"]         = t.plot;

        obj["director"]     = t.director;
        obj["actors"]       = t.actors;

        obj["totalSeasons"] = t.totalSeasons;

        arr.append(obj);
    }

    root["titles"] = arr;

    QFile file(appFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "Failed to open file for writing:" << appFilePath;
        return;
    }

    QJsonDocument doc(root);

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void AppStorage::setOmdbApiKey(QString key)
{
    omdbApiKey = key;

    QFile file(appFilePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file for updating API key:" << appFilePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    root["omdbApiKey"] = key;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "Failed to write file:" << appFilePath;
        return;
    }

    file.write(QJsonDocument(root).toJson());
    file.close();
}

void AppStorage::addTitle(const Title &title, const QPixmap &posterImage)
{
    auto it = std::find_if(
        titles.begin(),
        titles.end(),
        [&](const Title &t)
        {
            return t.imdbId == title.imdbId;
        });

    if (it != titles.end())
        return;

    QString posterFile =
        postersPath + "/" + title.imdbId + ".png";

    posterImage.save(posterFile, "PNG");

    titles.push_back(title);

    save();
    emit titlesUpdated();
}

void AppStorage::deleteTitle(const QString &imdbId)
{
    auto it = std::find_if(
        titles.begin(),
        titles.end(),
        [&](const Title &t)
        {
            return t.imdbId == imdbId;
        });

    if (it != titles.end())
    {
        QFile::remove(postersPath + "/" + imdbId + ".png");

        titles.erase(it);

        save();
        emit titlesUpdated();
    }
}

bool AppStorage::contains(const QString &imdbId) const
{
    return std::any_of(
        titles.begin(),
        titles.end(),
        [&](const Title &t)
        {
            return t.imdbId == imdbId;
        });
}