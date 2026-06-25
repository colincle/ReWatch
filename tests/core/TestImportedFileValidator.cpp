#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include "AppPaths.hpp"
#include "ImportedFileValidator.hpp"

class TestImportedFileValidator : public QObject
{
	Q_OBJECT

  private slots:
	void initTestCase() { QCoreApplication::setApplicationName("ReWatchTests"); }

	// validate()
	void validateNonExistentFileIsInvalid()
	{
		ValidationResult result =
		    ImportedFileValidator::validate("/nonexistent/path.zip");
		QVERIFY(!result.valid);
		QVERIFY(!result.error.isEmpty());
	}

	void validateMissingDataFileInZipIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";

		// Zip with a file that is NOT the expected data file
		writeFile(tmp.path() + "/unrelated.txt", "hello");
		QVERIFY(zipIt(tmp.path(), {"unrelated.txt"}, zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
	}

	void validateValidZipIsValid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), validJson(), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY2(result.valid, qPrintable(result.error));
	}

	void validateEmptyTitlesArrayIsValid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), jsonWith("titles", "[]"), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY2(result.valid, qPrintable(result.error));
	}

	void validateInvalidJsonIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), "not json at all", zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("Invalid JSON", Qt::CaseInsensitive));
	}

	void validateJsonArrayAtRootIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), "[]", zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("object", Qt::CaseInsensitive));
	}

	void validateMissingOmdbApiKeyIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), jsonWithout("omdbApiKey"), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("omdbApiKey"));
	}

	void validateMissingTitlesFieldIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), jsonWithout("titles"), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("titles"));
	}

	void validateTitleNotObjectIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), jsonWith("titles", "[42]"), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("not an object", Qt::CaseInsensitive));
	}

	void validateTitleMissingRankIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), jsonWithTitleMissing("rank"), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("rank"));
	}

	void validateTitleMissingViewedIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(tmp.path(), jsonWithTitleMissing("viewed"), zipPath));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("viewed"));
	}

	void validateTitleInvalidTypeIsInvalid()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(
		    tmp.path(),
		    jsonWithTitleField("type", "\"documentary\""),
		    zipPath
		));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY(!result.valid);
		QVERIFY(result.error.contains("type"));
	}

	void validateTitleTypeSeries()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(createLibraryZip(
		    tmp.path(),
		    jsonWithTitleField("type", "\"series\""),
		    zipPath
		));

		ValidationResult result = ImportedFileValidator::validate(zipPath);
		QVERIFY2(result.valid, qPrintable(result.error));
	}

	void validateRequiredStringFields()
	{
		const QStringList fields = {
		    "actors",
		    "director",
		    "imdbId",
		    "lastChecked",
		    "lastViewed",
		    "plot",
		    "released",
		    "title",
		    "type"
		};

		for(const QString &field : fields)
		{
			QTemporaryDir tmp;
			QVERIFY(tmp.isValid());
			QString zipPath = tmp.path() + "/test.zip";
			QVERIFY(createLibraryZip(tmp.path(), jsonWithTitleMissing(field), zipPath));

			ValidationResult result = ImportedFileValidator::validate(zipPath);
			QVERIFY2(
			    !result.valid,
			    qPrintable("Expected failure for missing field: " + field)
			);
			QVERIFY2(
			    result.error.contains(field),
			    qPrintable("Error should mention: " + field)
			);
		}
	}

	// entriesAreSafe()
	void entriesAreSafeNonExistentFileReturnsFalse()
	{
		QVERIFY(!ImportedFileValidator::entriesAreSafe("/nonexistent/path.zip"));
	}

	void entriesAreSafeSafeZipReturnsTrue()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QString zipPath = tmp.path() + "/test.zip";
		writeFile(tmp.path() + "/safe.txt", "hello");
		QVERIFY(zipIt(tmp.path(), {"safe.txt"}, zipPath));

		QVERIFY(ImportedFileValidator::entriesAreSafe(zipPath));
	}

	void entriesAreSafeNestedSafePathReturnsTrue()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		QDir(tmp.path()).mkpath("subdir");
		writeFile(tmp.path() + "/subdir/file.txt", "hello");
		QString zipPath = tmp.path() + "/test.zip";
		QVERIFY(zipIt(tmp.path(), {"subdir/file.txt"}, zipPath));

		QVERIFY(ImportedFileValidator::entriesAreSafe(zipPath));
	}

  private:
	static QString dirName() { return QDir(AppPaths::dataDir()).dirName(); }

	static void writeFile(const QString &path, const QString &content)
	{
		QFile f(path);
		(void)f.open(QFile::WriteOnly);
		f.write(content.toUtf8());
	}

	static bool
	zipIt(const QString &workingDir, const QStringList &entries, const QString &zipPath)
	{
		QProcess p;
		p.setWorkingDirectory(workingDir);
		QStringList args = {zipPath};
		args << entries;
		p.start("zip", args);
		return p.waitForFinished(5000) && p.exitCode() == 0;
	}

	static bool createLibraryZip(
	    const QString &tmpBase, const QString &jsonContent, const QString &zipPath
	)
	{
		const QString name = dirName();
		QDir(tmpBase).mkpath(name);
		writeFile(tmpBase + "/" + name + "/" + AppPaths::dataFile, jsonContent);
		return zipIt(tmpBase, {name + "/" + AppPaths::dataFile}, zipPath);
	}

	static QString validTitle()
	{
		return R"({
            "actors": "Some Actor",
            "director": "Some Director",
            "imdbId": "tt0000001",
            "lastChecked": "2024-01-01",
            "lastViewed": "2024-01-01",
            "plot": "A plot.",
            "released": "01 Jan 2024",
            "title": "Test Movie",
            "type": "movie",
            "rank": 0,
            "viewed": false,
            "posterNotFound": false
        })";
	}

	static QString validJson()
	{
		return R"({"omdbApiKey": "testkey", "titles": [)" + validTitle() + "]}";
	}

	// Replace a top-level field value
	static QString jsonWith(const QString &field, const QString &value)
	{
		if(field == "titles")
			return R"({"omdbApiKey": "testkey", "titles": )" + value + "}";
		return R"({"omdbApiKey": "testkey", "titles": [)" + validTitle() + "]}";
	}

	// Remove a top-level field
	static QString jsonWithout(const QString &field)
	{
		if(field == "omdbApiKey")
			return R"({"titles": [)" + validTitle() + "]}";
		if(field == "titles")
			return R"({"omdbApiKey": "testkey"})";
		return validJson();
	}

	// Remove a field from the title object
	static QString jsonWithTitleMissing(const QString &field)
	{
		QString title = validTitle();
		// Remove the line containing the field
		QStringList lines = title.split('\n');
		lines.erase(
		    std::remove_if(
		        lines.begin(),
		        lines.end(),
		        [&](const QString &line) { return line.contains("\"" + field + "\""); }
		    ),
		    lines.end()
		);
		return R"({"omdbApiKey": "testkey", "titles": [)" + lines.join('\n') + "]}";
	}

	// Replace a field value in the title object
	static QString jsonWithTitleField(const QString &field, const QString &value)
	{
		QString     title = validTitle();
		QStringList lines = title.split('\n');
		for(QString &line : lines)
		{
			if(line.contains("\"" + field + "\""))
			{
				int colon = line.indexOf(':');
				if(colon != -1)
					line = line.left(colon + 1) + " " + value + ",";
			}
		}
		return R"({"omdbApiKey": "testkey", "titles": [)" + lines.join('\n') + "]}";
	}
};

#include "TestImportedFileValidator.moc"

QObject *createTestImportedFileValidator()
{
	return new TestImportedFileValidator();
}
