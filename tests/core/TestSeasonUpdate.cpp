#include <algorithm>

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "SeasonUpdate.hpp"
#include "Title.hpp"

class TestSeasonUpdate : public QObject
{
	Q_OBJECT

  private:
	static QString apiKey() { return qEnvironmentVariable("OMDB_API_KEY"); }
	static bool    hasApiKey() { return !apiKey().isEmpty(); }

	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static Title makeSeries(
	    const QString &imdbId, QDate lastChecked = QDate{},
	    const QString &totalSeasons = "3"
	)
	{
		Title t;
		t.imdbId = imdbId;
		t.title = "Test Series";
		t.type = "series";
		t.totalSeasons = totalSeasons;
		t.lastChecked = lastChecked;
		return t;
	}

	static Title makeMovie(const QString &imdbId = "tt0000001")
	{
		Title t;
		t.imdbId = imdbId;
		t.title = "Test Movie";
		t.type = "movie";
		return t;
	}

  private slots:
	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		QDir(testDataDir()).removeRecursively();
	}

	void cleanupTestCase() { QDir(testDataDir()).removeRecursively(); }
	void init() { QDir(testDataDir()).removeRecursively(); }

	// -- isEmpty / eligibility -------------------------------------------------

	void isEmptyWithNoTitlesInLibrary()
	{
		AppStorage   storage;
		SeasonUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void isEmptyWithOnlyMovies()
	{
		AppStorage storage;
		storage.addTitle(makeMovie(), QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void isEmptyWhenAllSeriesCheckedToday()
	{
		AppStorage storage;
		storage.addTitle(makeSeries("tt0000001", QDate::currentDate()), QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void isNotEmptyWhenSeriesNeverChecked()
	{
		AppStorage storage;
		storage.addTitle(makeSeries("tt0000001", QDate{}), QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	void isNotEmptyWhenSeriesCheckedYesterday()
	{
		AppStorage storage;
		storage.addTitle(
		    makeSeries("tt0000001", QDate::currentDate().addDays(-1)),
		    QPixmap{}
		);
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	void moviesAreNeverEligible()
	{
		AppStorage storage;
		storage.addTitle(makeMovie("tt0000001"), QPixmap{});
		storage.addTitle(makeMovie("tt0000002"), QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void onlyUncheckedSeriesAreQueued()
	{
		AppStorage storage;
		storage.addTitle(
		    makeSeries("tt0000001", QDate::currentDate()),
		    QPixmap{}
		); // not eligible: checked today
		storage.addTitle(
		    makeSeries("tt0000002", QDate{}),
		    QPixmap{}
		);                                                   // eligible: never checked
		storage.addTitle(makeMovie("tt0000003"), QPixmap{}); // not eligible: movie
		storage.addTitle(
		    makeSeries("tt0000004", QDate::currentDate().addDays(-1)),
		    QPixmap{}
		); // eligible: checked yesterday
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	// -- updateSeries() integration --------------------------------------------

	void updateSeriesEmitsSeriesUpdated()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(
		    makeSeries("tt0944947", QDate{}, "8"),
		    QPixmap{}
		); // Game of Thrones
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
		QSignalSpy spy(&su, &SeasonUpdate::seriesUpdated);
		su.updateSeries();
		QCOMPARE(spy.count(), 1);
	}

	void updateSeriesUpdatesLastCheckedToToday()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(
		    makeSeries("tt0944947", QDate::currentDate().addDays(-1), "8"),
		    QPixmap{}
		);
		SeasonUpdate su(storage);
		su.updateSeries();
		const auto &titles = storage.getTitles();
		auto        it = std::find_if(
		    titles.begin(),
		    titles.end(),
		    [](const Title &t) { return t.imdbId == "tt0944947"; }
		);
		QVERIFY(it != titles.end());
		QCOMPARE(it->lastChecked, QDate::currentDate());
	}

	void updateSeriesDetectsNewSeasonsAndAddsNotification()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		// Game of Thrones has 8 seasons (ended) - set to 1 so new seasons are detected
		storage.addTitle(makeSeries("tt0944947", QDate{}, "1"), QPixmap{});
		SeasonUpdate su(storage);
		su.updateSeries();
		const auto &titles = storage.getTitles();
		auto        it = std::find_if(
		    titles.begin(),
		    titles.end(),
		    [](const Title &t) { return t.imdbId == "tt0944947"; }
		);
		QVERIFY(it != titles.end());
		QCOMPARE(it->totalSeasons, "8");
		QVERIFY(!it->viewed);
		const auto &notifications = storage.getNotifications();
		QVERIFY(
		    std::find(notifications.begin(), notifications.end(), QString("tt0944947")) !=
		    notifications.end()
		);
	}

	void updateSeriesNoNotificationWhenSeasonCountUnchanged()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		// Game of Thrones already at 8 seasons (final count)
		storage.addTitle(makeSeries("tt0944947", QDate{}, "8"), QPixmap{});
		SeasonUpdate su(storage);
		su.updateSeries();
		QVERIFY(storage.getNotifications().empty());
	}

	void updateSeriesWithInvalidKeyEmitsApiKeyError()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
		QSignalSpy spy(&su, &SeasonUpdate::apiKeyError);
		su.updateSeries();
		QCOMPARE(spy.count(), 1);
	}
};

#include "TestSeasonUpdate.moc"

QObject *createTestSeasonUpdate()
{
	return new TestSeasonUpdate();
}
