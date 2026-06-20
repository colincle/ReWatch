#pragma once

#include "Palette.hpp"
#include "Title.hpp"

#include <vector>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QMutexLocker>

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
	class LockGuard
	{
		friend class AppStorage;
		explicit LockGuard(QRecursiveMutex &m) : locker(&m) {}
		QMutexLocker<QRecursiveMutex> locker;

	  public:
		LockGuard(LockGuard &&) = default;
	};

	AppStorage();

	void setOmdbApiKey(QString key);
	void addTitle(const Title &title, const QPixmap &posterImage);
	void deleteTitle(const QString &imdbId);
	void toggleViewed(const QString &imdbId);
	void setPoster(const QString &imdbId, const QPixmap &image);
	void setLibraryCardWidth(int width);
	void setTheme(QString theme);
	void setDarkAccentColor(const QString &color);
	void setLightAccentColor(const QString &color);
	void setWindowSize(int width, int height);
	void addStreamingPlatform(StreamingPlatform platform, const QString &sourceImagePath);
	void removeStreamingPlatform(const QString &name);
	bool importFrom(const QString &zipPath);
	bool exportTo(const QString &zipPath);

	bool contains(const QString &imdbId) const;

	void save();

	void addNotifications(const std::vector<QString> &values);
	void removeNotifications();
	void insertRank(const QString &imdbId, int position, const QString &type);
	void resetRankings(const QString &type);
	void clearRank(const QString &imdbId);

	int     getLibraryCardWidth() const { return libraryCardWidth; }
	QString getTheme() const { return theme; }
	QString getDarkAccentColor() const { return darkAccentColor; }
	QString getLightAccentColor() const { return lightAccentColor; }
	QString getAccentColor() const
	{
		return theme == "light" ? lightAccentColor : darkAccentColor;
	}
	WindowSize                  getWindowSize() const { return windowSize; }
	const std::vector<Title>   &getTitles() const { return titles; }
	[[nodiscard]] LockGuard     lock() { return LockGuard(mutex); }
	std::vector<Title>         &getTitlesMutable(LockGuard &) { return titles; }
	const std::vector<QString> &getNotifications() const { return notifications; }
	const std::vector<StreamingPlatform> &getStreamingPlatforms() const
	{
		return streamingPlatforms;
	}
	QString getKey() const;

  private:
	QString                        appFilePath;
	QString                        omdbApiKey;
	QString                        theme;
	QString                        darkAccentColor = Palette::defaultAccent;
	QString                        lightAccentColor = Palette::defaultAccent;
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
	void styleChanged();
};