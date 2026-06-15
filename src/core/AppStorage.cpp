#include "AppStorage.hpp"
#include "AssetsPaths.hpp"

#include <QDir>
#include <QFile>
#include <QDate>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static QString appDirPath()
{
	return QDir::homePath() + "/.local/share/movieTracker";
}

static void ensureDirectoryExists(const QString &path)
{
	QDir dir(path);

	if(!dir.exists())
	{
		dir.mkpath(path);
	}
}

static void ensureStorageFileExists(const QString &filePath)
{
	QFile file(filePath);

	if(file.exists())
	{
		return;
	}

	if(file.open(QIODevice::WriteOnly))
	{
		file.write("{\n\t\"omdbApiKey\": \"\",\n\t\"titles\": []\n}\n");
		file.close();
	}
}

static QJsonObject readJsonFile(const QString &filePath)
{
	QFile file(filePath);

	if(!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to open file for reading:" << filePath;
		return {};
	}

	QByteArray data = file.readAll();
	file.close();

	return QJsonDocument::fromJson(data).object();
}

static bool writeJsonFile(const QString &filePath, const QJsonObject &root)
{
	QFile file(filePath);

	if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qWarning() << "Failed to open file for writing:" << filePath;
		return false;
	}

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();
	return true;
}

static Title titleFromJson(const QJsonObject &obj)
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

static QJsonObject titleToJson(const Title &t)
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

	obj["rank"] = t.rank;
	obj["viewed"] = t.viewed;
	obj["lastViewed"] = t.lastViewed.toString(Qt::ISODate);
	obj["lastChecked"] = t.lastChecked.toString(Qt::ISODate);

	return obj;
}

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

AppStorage::AppStorage()
{
	const QString dirPath = appDirPath();
	postersPath = dirPath + "/Posters";
	appFilePath = dirPath + "/movieTracker.json";

	ensureDirectoryExists(dirPath);
	ensureDirectoryExists(postersPath);
	ensureStorageFileExists(appFilePath);

	load();
}

void AppStorage::load()
{
	QJsonObject root = readJsonFile(appFilePath);

	if(root.isEmpty())
	{
		return;
	}

	omdbApiKey = root["omdbApiKey"].toString();
	titles.clear();

	for(const QJsonValue &val : root["titles"].toArray())
	{
		Title t = titleFromJson(val.toObject());

		if(!t.posterImage.load(posterPath(postersPath, t.imdbId)))
		{
			t.posterImage.load(POSTER_PLACEHOLDER);
		}

		titles.push_back(std::move(t));
	}
}

void AppStorage::save()
{
	QJsonArray arr;

	for(const Title &t : titles)
	{
		arr.append(titleToJson(t));
	}

	QJsonObject root;
	root["omdbApiKey"] = omdbApiKey;
	root["titles"] = arr;

	writeJsonFile(appFilePath, root);
}

void AppStorage::setOmdbApiKey(QString key)
{
	omdbApiKey = key;
	save();
}

void AppStorage::addTitle(const Title &title, const QPixmap &posterImage)
{
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

bool AppStorage::contains(const QString &imdbId) const
{
	return std::any_of(
	       titles.begin(),
	titles.end(),
	[&](const Title & t) { return t.imdbId == imdbId; });
}