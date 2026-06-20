#include "AppStorage.hpp"
#include "AppPaths.hpp"
#include "AssetsPaths.hpp"
#include "ImportedFileValidator.hpp"
#include "JsonFileIO.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutexLocker>
#include <QProcess>

static QString posterPath(const QString &postersPath, const QString &imdbId)
{
	return postersPath + "/" + imdbId + ".png";
}

static auto findByImdbId(std::vector<Title> &titles, const QString &imdbId)
{
	return std::find_if(
	    titles.begin(),
	    titles.end(),
	    [&](const Title &t) { return t.imdbId == imdbId; }
	);
}

Title AppStorage::titleFromStorageJson(const QJsonObject &obj) const
{
	Title t;

	t.title = obj["title"].toString();
	t.year = obj["year"].toString();
	t.imdbId = obj["imdbId"].toString();
	t.type = obj["type"].toString();

	t.released = obj["released"].toString();
	t.plot = obj["plot"].toString();

	t.director = obj["director"].toString();
	t.actors = obj["actors"].toString();

	t.totalSeasons = obj["totalSeasons"].toString();

	t.posterNotFound = obj["posterNotFound"].toBool(false);

	t.rank = obj["rank"].toInt(0);
	t.viewed = obj["viewed"].toBool(false);

	t.lastViewed = QDate::fromString(obj["lastViewed"].toString(), Qt::ISODate);
	t.lastChecked = QDate::fromString(obj["lastChecked"].toString(), Qt::ISODate);

	return t;
}

QJsonObject AppStorage::titleToStorageJson(const Title &t) const
{
	QJsonObject obj;

	obj["title"] = t.title;
	obj["year"] = t.year;
	obj["imdbId"] = t.imdbId;
	obj["type"] = t.type;

	obj["released"] = t.released;
	obj["plot"] = t.plot;

	obj["director"] = t.director;
	obj["actors"] = t.actors;

	obj["totalSeasons"] = t.totalSeasons;
	obj["posterNotFound"] = t.posterNotFound;

	obj["rank"] = t.rank;
	obj["viewed"] = t.viewed;
	obj["lastViewed"] = t.lastViewed.toString(Qt::ISODate);
	obj["lastChecked"] = t.lastChecked.toString(Qt::ISODate);

	return obj;
}

AppStorage::AppStorage()
{
	const QString dirPath = AppPaths::dataDir();
	postersPath = dirPath + "/" + AppPaths::postersDir;
	platformImagesPath = dirPath + "/" + AppPaths::platformImagesDir;
	appFilePath = dirPath + "/" + AppPaths::dataFile;

	ensureDirectoryExists(dirPath);
	ensureDirectoryExists(postersPath);
	ensureDirectoryExists(platformImagesPath);
	ensureStorageFileExists(appFilePath);

	load();
}

void AppStorage::load()
{
	QMutexLocker locker(&mutex);

	QJsonObject root = readJsonFile(appFilePath);

	if(root.isEmpty())
	{
		return;
	}

	windowSize.width = root["windowWidth"].toInt(1200);
	windowSize.height = root["windowHeight"].toInt(800);
	omdbApiKey = root["omdbApiKey"].toString();
	theme = root["theme"].toString("dark");
	libraryCardWidth = root["libraryCardWidth"].toInt(160);
	titles.clear();
	notifications.clear();
	streamingPlatforms.clear();

	for(const QJsonValue &val : root["titles"].toArray())
	{
		Title t = titleFromStorageJson(val.toObject());

		if(!t.posterImage.load(posterPath(postersPath, t.imdbId)))
		{
			t.posterImage.load(AssetsPaths::posterPlaceholder);
		}

		titles.push_back(std::move(t));
	}

	for(const QJsonValue &val : root["notifications"].toArray())
	{
		notifications.push_back(val.toString());
	}

	for(const QJsonValue &val : root["streamingPlatforms"].toArray())
	{
		QJsonObject   obj = val.toObject();
		const QString imageName = obj["image"].toString();
		streamingPlatforms.push_back(
		    {obj["url"].toString(),
		     obj["name"].toString(),
		     imageName.isEmpty() ? QString{} : platformImagesPath + "/" + imageName}
		);
	}
}

void AppStorage::save()
{
	QMutexLocker locker(&mutex);

	QJsonArray arr;

	for(const Title &t : titles)
	{
		arr.append(titleToStorageJson(t));
	}

	QJsonArray notificationsArr;

	for(const QString &n : notifications)
	{
		notificationsArr.append(n);
	}

	QJsonArray platformsArr;

	for(const StreamingPlatform &p : streamingPlatforms)
	{
		QJsonObject obj;
		obj["url"] = p.url;
		obj["name"] = p.name;
		obj["image"] = QFileInfo(p.image).fileName();
		platformsArr.append(obj);
	}

	QJsonObject root;
	root["windowWidth"] = windowSize.width;
	root["windowHeight"] = windowSize.height;
	root["libraryCardWidth"] = libraryCardWidth;
	root["theme"] = theme;
	root["omdbApiKey"] = omdbApiKey;
	root["notifications"] = notificationsArr;
	root["streamingPlatforms"] = platformsArr;
	root["titles"] = arr;

	if(!writeJsonFile(appFilePath, root))
	{
		emit saveFailed();
	}
}

bool AppStorage::exportTo(const QString &zipPath)
{
	QMutexLocker locker(&mutex);

	QDir     appDir(AppPaths::dataDir());
	QProcess process;
	process.setWorkingDirectory(appDir.absolutePath() + "/..");
	process.start("zip", {"-r", zipPath, appDir.dirName()});
	return process.waitForFinished(10000) && process.exitCode() == 0;
}

bool AppStorage::importFrom(const QString &zipPath)
{
	QMutexLocker locker(&mutex);

	if(!ImportedFileValidator::entriesAreSafe(zipPath))
	{
		return false;
	}

	QDir     appDir(AppPaths::dataDir());
	QProcess process;
	process.setWorkingDirectory(appDir.absolutePath() + "/..");
	process.start("unzip", {"-o", zipPath, "-d", "."});

	if(!process.waitForFinished(10000) || process.exitCode() != 0)
	{
		return false;
	}

	load();
	emit titlesUpdated();
	return true;
}

void AppStorage::setOmdbApiKey(QString key)
{
	QMutexLocker locker(&mutex);

	omdbApiKey = key;
	save();
	emit apiKeyChanged();
}

void AppStorage::setWindowSize(int width, int height)
{
	QMutexLocker locker(&mutex);

	windowSize.width = width;
	windowSize.height = height;
	save();
}

void AppStorage::setLibraryCardWidth(int width)
{
	QMutexLocker locker(&mutex);

	libraryCardWidth = width;
	save();
}

void AppStorage::setTheme(QString newTheme)
{
	QMutexLocker locker(&mutex);

	theme = newTheme;
	save();
}

void AppStorage::addTitle(const Title &title, const QPixmap &posterImage)
{
	QMutexLocker locker(&mutex);

	if(contains(title.imdbId))
	{
		return;
	}

	posterImage.save(posterPath(postersPath, title.imdbId), "PNG");

	Title t = title;
	t.viewed = false;
	titles.push_back(std::move(t));

	save();
	emit titlesUpdated();
}

void AppStorage::deleteTitle(const QString &imdbId)
{
	QMutexLocker locker(&mutex);

	auto it = findByImdbId(titles, imdbId);

	if(it == titles.end())
	{
		return;
	}

	QFile::remove(posterPath(postersPath, imdbId));
	titles.erase(it);

	save();
	emit titlesUpdated();
}

void AppStorage::toggleViewed(const QString &imdbId)
{
	QMutexLocker locker(&mutex);

	auto it = findByImdbId(titles, imdbId);

	if(it == titles.end())
	{
		return;
	}

	it->viewed = !it->viewed;

	if(it->viewed)
	{
		it->lastViewed = QDate::currentDate();
	}

	save();
	emit titlesUpdated();
}

void AppStorage::setPoster(const QString &imdbId, const QPixmap &image)
{
	QMutexLocker locker(&mutex);

	auto it = findByImdbId(titles, imdbId);

	if(it == titles.end())
	{
		return;
	}

	image.save(posterPath(postersPath, imdbId), "PNG");

	it->posterImage = image;

	save();
	emit titlesUpdated();
}

void AppStorage::addNotifications(const std::vector<QString> &values)
{
	QMutexLocker locker(&mutex);

	bool added = false;

	for(const QString &value : values)
	{
		if(std::find(notifications.begin(), notifications.end(), value) ==
		   notifications.end())
		{
			notifications.push_back(value);
			added = true;
		}
	}

	save();
	emit notificationsChanged();

	if(added)
	{
		emit notificationsAdded();
	}
}

void AppStorage::removeNotifications()
{
	QMutexLocker locker(&mutex);

	notifications.clear();
	save();
	emit notificationsChanged();
}

void AppStorage::clearRank(const QString &imdbId)
{
	QMutexLocker locker(&mutex);

	auto it = findByImdbId(titles, imdbId);
	if(it == titles.end() || it->rank == 0)
		return;

	const int     removedRank = it->rank;
	const QString type = it->type;

	it->rank = 0;

	for(Title &t : titles)
		if(t.type == type && t.rank > removedRank)
			t.rank--;

	save();
	emit titlesUpdated();
}

void AppStorage::resetRankings(const QString &type)
{
	QMutexLocker locker(&mutex);

	for(Title &t : titles)
		if(t.type == type)
			t.rank = 0;

	save();
	emit titlesUpdated();
}

void AppStorage::insertRank(const QString &imdbId, int position, const QString &type)
{
	QMutexLocker locker(&mutex);

	const int newRank = position + 1;

	for(Title &t : titles)
	{
		if(t.type == type && t.rank >= newRank)
			t.rank++;
	}

	auto it = findByImdbId(titles, imdbId);
	if(it != titles.end())
		it->rank = newRank;

	save();
	emit titlesUpdated();
}

void AppStorage::addStreamingPlatform(
    StreamingPlatform platform, const QString &sourceImagePath
)
{
	QMutexLocker locker(&mutex);

	if(!sourceImagePath.isEmpty())
	{
		const QString ext = QFileInfo(sourceImagePath).suffix();
		const QString destName = platform.name + (ext.isEmpty() ? QString{} : "." + ext);
		const QString destPath = platformImagesPath + "/" + destName;
		QFile::copy(sourceImagePath, destPath);
		platform.image = destPath;
	}

	streamingPlatforms.push_back(std::move(platform));
	save();
	emit streamingPlatformsChanged();
}

void AppStorage::removeStreamingPlatform(const QString &name)
{
	QMutexLocker locker(&mutex);

	auto it = std::find_if(
	    streamingPlatforms.begin(),
	    streamingPlatforms.end(),
	    [&](const StreamingPlatform &p) { return p.name == name; }
	);

	if(it == streamingPlatforms.end())
	{
		return;
	}

	if(!it->image.isEmpty())
		QFile::remove(it->image);
	streamingPlatforms.erase(it);
	save();
	emit streamingPlatformsChanged();
}

QString AppStorage::getKey() const
{
	QMutexLocker locker(&mutex);
	return omdbApiKey;
}

bool AppStorage::contains(const QString &imdbId) const
{
	QMutexLocker locker(&mutex);

	return std::any_of(
	    titles.begin(),
	    titles.end(),
	    [&](const Title &t) { return t.imdbId == imdbId; }
	);
}
