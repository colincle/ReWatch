#include "AppStorage.hpp"
#include "AppPaths.hpp"
#include "AssetsPaths.hpp"
#include "ImportedFileValidator.hpp"
#include "JsonFileIO.hpp"

#include <QDir>
#include <QFile>
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
	[&](const Title & t) { return t.imdbId == imdbId; });
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

	t.isMovie = t.type == "movie";
	t.isSeries = t.type == "series";

	t.rank = obj["rank"].toInt(0);
	t.viewed = obj["viewed"].toBool(false);

	t.lastViewed = QDate::fromString(obj["lastViewed"].toString(), Qt::ISODate);

	if(!t.lastViewed.isValid())
	{
		t.lastViewed = QDate::currentDate();
	}

	t.lastChecked = QDate::fromString(obj["lastChecked"].toString(), Qt::ISODate);

	if(!t.lastChecked.isValid())
	{
		t.lastChecked = QDate::currentDate();
	}

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
	const QString dirPath = APP_DATA_DIR;
	postersPath = dirPath + "/" APP_POSTERS_DIR;
	appFilePath = dirPath + "/" APP_DATA_FILE;

	ensureDirectoryExists(dirPath);
	ensureDirectoryExists(postersPath);
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

	omdbApiKey = root["omdbApiKey"].toString();
	titles.clear();

	for(const QJsonValue &val : root["titles"].toArray())
	{
		Title t = titleFromStorageJson(val.toObject());

		if(!t.posterImage.load(posterPath(postersPath, t.imdbId)))
		{
			t.posterImage.load(POSTER_PLACEHOLDER);
			t.posterNotFound = true;
		}

		titles.push_back(std::move(t));
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

	QJsonObject root;
	root["omdbApiKey"] = omdbApiKey;
	root["titles"] = arr;

	if(!writeJsonFile(appFilePath, root))
	{
		emit saveFailed();
	}
}

bool AppStorage::exportTo(const QString &zipPath)
{
	QMutexLocker locker(&mutex);

	QDir appDir(APP_DATA_DIR);
	QProcess process;
	process.setWorkingDirectory(appDir.absolutePath() + "/..");
	process.start("zip", { "-r", zipPath, appDir.dirName() });
	return process.waitForFinished(10000) && process.exitCode() == 0;
}

bool AppStorage::importFrom(const QString &zipPath)
{
	QMutexLocker locker(&mutex);

	if(!ImportedFileValidator::entriesAreSafe(zipPath))
	{
		return false;
	}

	QDir appDir(APP_DATA_DIR);
	QProcess process;
	process.setWorkingDirectory(appDir.absolutePath() + "/..");
	process.start("unzip", { "-o", zipPath, "-d", "." });

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
	it->posterNotFound = false;

	save();
	emit titlesUpdated();
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
	[&](const Title & t) { return t.imdbId == imdbId; });
}
