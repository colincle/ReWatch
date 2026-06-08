#pragma once

#include <QString>
#include <QList>

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
    QString posterUrl;

    // Series-specific (empty for movies)
    QString totalSeasons;

    // Utility helpers
    bool isMovie;
    bool isSeries;
};