#pragma once

#include "Title.hpp"

#include <vector>
#include <QObject>
#include <QPixmap>
#include <QString>

class AppStorage : public QObject
{
	Q_OBJECT

public:
	AppStorage();

	void setOmdbApiKey(QString key);
	void addTitle(const Title &title, const QPixmap &posterImage);
	void deleteTitle(const QString &imdbId);
	void toggleViewed(const QString &imdbId);

	bool contains(const QString &imdbId) const;

	void save();

	const std::vector<Title> &getTitles() const { return titles; }
	std::vector<Title> &getTitlesMutable() { return titles; }
	QString getKey() const { return omdbApiKey; }

private:
	QString appFilePath;
	QString omdbApiKey;
	QString postersPath;
	std::vector<Title> titles;

	void load();

signals:
	void titlesUpdated();
};