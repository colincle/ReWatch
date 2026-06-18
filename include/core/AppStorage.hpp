#pragma once

#include "Title.hpp"

#include <vector>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QString>

struct WindowSize
{
	int width;
	int height;
};

struct StreamingPlatform
{
	QString url;
	QString name;
	QString image;
};

class AppStorage : public QObject
{
	Q_OBJECT

  public:
	AppStorage();

	void setOmdbApiKey(QString key);
	void addTitle(const Title &title, const QPixmap &posterImage);
	void deleteTitle(const QString &imdbId);
	void toggleViewed(const QString &imdbId);
	void setPoster(const QString &imdbId, const QPixmap &image);
	void setLibraryCardWidth(int width);
	void setTheme(QString theme);
	void setWindowSize(int width, int height);
	void addStreamingPlatform(StreamingPlatform platform, const QString &sourceImagePath);
	void removeStreamingPlatform(const QString &name);
	bool importFrom(const QString &zipPath);
	bool exportTo(const QString &zipPath);

	bool contains(const QString &imdbId) const;

	void save();

	void addNotifications(const std::vector<QString> &values);
	void removeNotifications();

	int                         getLibraryCardWidth() const { return libraryCardWidth; }
	QString                     getTheme() const { return theme; }
	WindowSize                  getWindowSize() const { return windowSize; }
	const std::vector<Title>   &getTitles() const { return titles; }
	std::vector<Title>         &getTitlesMutable() { return titles; }
	const std::vector<QString> &getNotifications() const { return notifications; }
	const std::vector<StreamingPlatform> &getStreamingPlatforms() const
	{
		return streamingPlatforms;
	}
	QString getKey() const;

	QRecursiveMutex &getMutex() { return mutex; }

  private:
	QString                        appFilePath;
	QString                        omdbApiKey;
	QString                        theme;
	QString                        postersPath;
	QString                        platformImagesPath;
	int                            libraryCardWidth;
	WindowSize                     windowSize;
	std::vector<Title>             titles;
	std::vector<QString>           notifications;
	std::vector<StreamingPlatform> streamingPlatforms;
	mutable QRecursiveMutex        mutex;

	void        load();
	Title       titleFromStorageJson(const QJsonObject &obj) const;
	QJsonObject titleToStorageJson(const Title &t) const;

  signals:
	void titlesUpdated();
	void saveFailed();
	void apiKeyChanged();
	void notificationsChanged();
	void notificationsAdded();
	void streamingPlatformsChanged();
};