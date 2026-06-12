#pragma once

#include <QString>
#include <QList>
#include <QPixmap>

struct Title
{
    // Core identifiers
    QString title;
    QString year;
    QString imdbId;
    QString type; // "movie", "series", "episode"

    // Basic metadata
    QString released;
    QString plot;

    // People
    QString director;
    QString actors;

    // Media
    QPixmap posterImage;

    // Series-specific (empty for movies)
    QString totalSeasons;

    bool isMovie;
    bool isSeries;
};