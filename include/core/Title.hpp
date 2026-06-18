#pragma once

#include <QDate>
#include <QPixmap>
#include <QString>

struct Title
{
	QString title;
	QString year;
	QString imdbId;
	QString type;

	QString released;
	QString plot;

	QString director;
	QString actors;

	QString totalSeasons;

	QPixmap posterImage;
	bool    posterNotFound = false;

	int   rank = 0;
	bool  viewed = false;
	QDate lastViewed;
	QDate lastChecked;
};