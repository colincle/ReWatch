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
		auto        g = storage.lock();
		const auto &titles = storage.getTitles(g);
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
		{
			auto        g = storage.lock();
			const auto &titles = storage.getTitles(g);
			auto        it = std::find_if(
			    titles.begin(),
			    titles.end(),
			    [](const Title &t) { return t.imdbId == "tt0944947"; }
			);
			QVERIFY(it != titles.end());
			QCOMPARE(it->totalSeasons, "8");
			QVERIFY(!it->viewed);
		}
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

	// -- finished series interval -----------------------------------------------

	void finishedSeriesNotEligibleCheckedYesterday()
	{
		// Year "YYYY–YYYY" (en-dash) means the series ended → 30-day interval.
		// Checked yesterday (1 day ago) must not be eligible.
		AppStorage storage;
		Title      t = makeSeries("tt0000001", QDate::currentDate().addDays(-1));
		t.year = QString("2007") + QChar(0x2013) + "2013";
		storage.addTitle(t, QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void finishedSeriesEligibleAfterThirtyOneDays()
	{
		AppStorage storage;
		Title      t = makeSeries("tt0000001", QDate::currentDate().addDays(-31));
		t.year = QString("2007") + QChar(0x2013) + "2013";
		storage.addTitle(t, QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	void ongoingSeriesEligibleAfterOneDay()
	{
		// Year "YYYY–" (en-dash, no end year) means the series is ongoing → 1-day
		// interval.
		AppStorage storage;
		Title      t = makeSeries("tt0000001", QDate::currentDate().addDays(-1));
		t.year = QString("2019") + QChar(0x2013);
		storage.addTitle(t, QPixmap{});
		SeasonUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	// -- request cap and priority queue -----------------------------------------

	void requestCapLimitsQueueAndSavesOverflow()
	{
		AppStorage storage;
		storage.setMaxUpdateRequests(2);
		storage.addTitle(makeSeries("tt0000001", QDate{}), QPixmap{});
		storage.addTitle(makeSeries("tt0000002", QDate{}), QPixmap{});
		storage.addTitle(makeSeries("tt0000003", QDate{}), QPixmap{});
		storage.addTitle(makeSeries("tt0000004", QDate{}), QPixmap{});
		SeasonUpdate su(storage);
		auto         g = storage.lock();
		const auto  &overflow = storage.getUpdatePriority(g);
		QCOMPARE((int)overflow.size(), 2);
	}

	void priorityIdsProcessedBeforeOthers()
	{
		// With cap=1, the priority ID must be taken first so it is NOT in the overflow.
		AppStorage storage;
		storage.setMaxUpdateRequests(1);
		storage.setUpdatePriority({"tt0000003"});
		storage.addTitle(makeSeries("tt0000001", QDate{}), QPixmap{});
		storage.addTitle(makeSeries("tt0000002", QDate{}), QPixmap{});
		storage.addTitle(makeSeries("tt0000003", QDate{}), QPixmap{});
		SeasonUpdate su(storage);
		auto         g = storage.lock();
		const auto  &overflow = storage.getUpdatePriority(g);
		QCOMPARE((int)overflow.size(), 2);
		QVERIFY(
		    std::find(overflow.begin(), overflow.end(), QString("tt0000003")) ==
		    overflow.end()
		);
	}
};

#include "TestSeasonUpdate.moc"

QObject *createTestSeasonUpdate()
{
	return new TestSeasonUpdate();
}
