#pragma once

#include "Title.hpp"

#include <vector>
#include <QString>
#include <QObject>

class AppStorage : public QObject
{
    Q_OBJECT
public:
    AppStorage();
    void setOmdbApiKey(QString key);
    void addTitle(const Title &title, const QPixmap &posterImage);
    void deleteTitle(const QString &ImdbId);
    bool contains(const QString &imdbId) const;

    QString getKey() {return omdbApiKey;}

    private:
    QString appFilePath;
    QString omdbApiKey;
    QString postersPath;
    std::vector<Title> titles;

    void load();
    void save();

signals:
    void titlesUpdated();
};