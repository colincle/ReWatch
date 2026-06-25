// Central data type representing a movie or TV series in the user's library.
#pragma once

#include <QDate>
#include <QPixmap>
#include <QString>

struct LastEpisode
{
	int season = 1;
	int episode = 1;
};

struct Title
{
	QString title;
	QString imdbId;
	QString type;

	QString released;
	QString plot;

	QString director;
	QString actors;

	LastEpisode lastEpisode;
	QString     nextSeasonDate;

	QPixmap posterImage;
	bool    posterNotFound = false;

	int   rank = 0;
	bool  viewed = false;
	QDate lastViewed;
	QDate lastChecked;
};