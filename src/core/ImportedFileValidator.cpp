#include "ImportedFileValidator.hpp"
#include "AppPaths.hpp"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

static const QStringList REQUIRED_STRING_FIELDS = {
    "actors",
    "director",
    "imdbId",
    "lastChecked",
    "lastViewed",
    "plot",
    "released",
    "title",
    "totalSeasons",
    "type",
    "year"
};

static const QStringList VALID_TYPES = {"movie", "series"};

static ValidationResult validateJson(const QByteArray &data)
{
	QJsonParseError parseError;
	QJsonDocument   doc = QJsonDocument::fromJson(data, &parseError);

	if(doc.isNull())
		return {false, "Invalid JSON: " + parseError.errorString()};

	if(!doc.isObject())
		return {false, "Expected a JSON object at the root."};

	QJsonObject root = doc.object();

	if(!root.contains("omdbApiKey") || !root["omdbApiKey"].isString())
		return {false, "Missing or invalid field: omdbApiKey."};

	if(!root.contains("titles") || !root["titles"].isArray())
		return {false, "Missing or invalid field: titles."};

	QJsonArray titles = root["titles"].toArray();

	for(int i = 0; i < titles.size(); ++i)
	{
		if(!titles[i].isObject())
			return {false, QString("Title at index %1 is not an object.").arg(i)};

		QJsonObject obj = titles[i].toObject();

		auto err = [&](const QString &msg) -> ValidationResult
		{ return {false, QString("Title at index %1: %2").arg(i).arg(msg)}; };

		for(const QString &field : REQUIRED_STRING_FIELDS)
			if(!obj.contains(field) || !obj[field].isString())
			{
				return err("missing or invalid field: " + field);
			}

		if(!obj.contains("rank") || !obj["rank"].isDouble())
		{
			return err("missing or invalid field: rank");
		}

		if(!obj.contains("viewed") || !obj["viewed"].isBool())
		{
			return err("missing or invalid field: viewed");
		}

		if(!VALID_TYPES.contains(obj["type"].toString()))
		{
			return err("invalid type value: must be \"movie\" or \"series\"");
		}
	}

	return {true, {}};
}

ValidationResult ImportedFileValidator::validate(const QString &zipPath)
{
	QString  dirName = QDir(AppPaths::dataDir()).dirName();
	QProcess process;
	process.start("unzip", {"-p", zipPath, dirName + "/" + AppPaths::dataFile});

	if(!process.waitForFinished(5000))
		return {false, "Failed to read zip file."};

	if(process.exitCode() != 0)
		return {false, "Zip does not contain a valid ReWatch library."};

	return validateJson(process.readAllStandardOutput());
}

bool ImportedFileValidator::entriesAreSafe(const QString &zipPath)
{
	QProcess listProcess;
	listProcess.start("unzip", {"-Z1", zipPath});

	if(!listProcess.waitForFinished(10000) || listProcess.exitCode() != 0)
	{
		return false;
	}

	const QStringList entries = QString::fromUtf8(listProcess.readAllStandardOutput())
	                                .split('\n', Qt::SkipEmptyParts);

	for(const QString &entry : entries)
	{
		if(entry.startsWith('/') || entry.split('/', Qt::SkipEmptyParts).contains(".."))
		{
			return false;
		}
	}

	return true;
}
