#include "AppUtils.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AppUtils::AppUtils()
{
    QString dirPath = QDir::homePath() + "/.local/share/movieTracker";
    QString filePath = dirPath + "/movieTracker.json";

    appFilePath = filePath;

    QDir dir(dirPath);
    if (!dir.exists())
        dir.mkpath(dirPath);

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

void AppUtils::load()
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
        t.title = obj["title"].toString();
        t.year = obj["year"].toString();
        t.imdbId = obj["imdbId"].toString();
        t.type = obj["type"].toString();

        t.released = obj["released"].toString();
        t.plot = obj["plot"].toString();

        t.director = obj["director"].toString();
        t.actors = obj["actors"].toString();

        t.posterUrl = obj["posterUrl"].toString();
        t.totalSeasons = obj["totalSeasons"].toString();

        titles.push_back(t);
    }
}

void AppUtils::setOmdbApiKey(QString key)
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