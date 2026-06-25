#include <algorithm>

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "LibraryUpdate.hpp"
#include "Title.hpp"

class TestLibraryUpdate : public QObject
{
	Q_OBJECT

  private:
	static QString apiKey() { return qEnvironmentVariable("OMDB_API_KEY"); }
	static bool    hasApiKey() { return !apiKey().isEmpty(); }

	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static Title
	makeSeries(const QString &imdbId, QDate lastChecked = QDate{}, int seasons = 3)
	{
		Title t;
		t.imdbId = imdbId;
		t.title = "Test Series";
		t.type = "series";
		t.lastEpisode.season = seasons;
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
		AppStorage    storage;
		LibraryUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void isEmptyWithOnlyMovies()
	{
		AppStorage storage;
		storage.addTitle(makeMovie(), QPixmap{});
		LibraryUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void isEmptyWhenAllSeriesCheckedToday()
	{
		AppStorage storage;
		storage.addTitle(makeSeries("tt0000001", QDate::currentDate()), QPixmap{});
		LibraryUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void isNotEmptyWhenSeriesNeverChecked()
	{
		AppStorage storage;
		storage.addTitle(makeSeries("tt0000001", QDate{}), QPixmap{});
		LibraryUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	void isNotEmptyWhenSeriesCheckedYesterday()
	{
		AppStorage storage;
		storage.addTitle(
		    makeSeries("tt0000001", QDate::currentDate().addDays(-1)),
		    QPixmap{}
		);
		LibraryUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	void moviesAreNeverEligible()
	{
		AppStorage storage;
		storage.addTitle(makeMovie("tt0000001"), QPixmap{});
		storage.addTitle(makeMovie("tt0000002"), QPixmap{});
		LibraryUpdate su(storage);
		QVERIFY(su.isEmpty());
	}

	void upcomingMovieWithFutureDateIsNotQueued()
	{
		// Movies in upcomingMovies are only checked once their stored release date
		// arrives.
		AppStorage storage;
		Title      t = makeMovie("tt0000001");
		t.released = QDate::currentDate().addDays(30).toString("dd MMM yyyy");
		storage.addTitle(t, QPixmap{});
		// addTitle puts it in upcomingMovies, but LibraryUpdate must not queue it yet.
		LibraryUpdate su(storage);
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
		LibraryUpdate su(storage);
		QVERIFY(!su.isEmpty());
	}

	// -- run() integration --------------------------------------------

	void runEmitsFinished()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(
		    makeSeries("tt0944947", QDate{}, 8),
		    QPixmap{}
		); // Game of Thrones
		LibraryUpdate su(storage);
		QVERIFY(!su.isEmpty());
		QSignalSpy spy(&su, &LibraryUpdate::finished);
		su.run();
		QCOMPARE(spy.count(), 1);
	}

	void runUpdatesLastCheckedToToday()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(
		    makeSeries("tt0944947", QDate::currentDate().addDays(-1), 8),
		    QPixmap{}
		);
		LibraryUpdate su(storage);
		su.run();
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

	void runDetectsNewSeasonsAndAddsNotification()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		// Game of Thrones has 8 seasons (ended) - set to 1 so new seasons are detected
		storage.addTitle(makeSeries("tt0944947", QDate{}, 1), QPixmap{});
		LibraryUpdate su(storage);
		su.run();
		{
			auto        g = storage.lock();
			const auto &titles = storage.getTitles(g);
			auto        it = std::find_if(
			    titles.begin(),
			    titles.end(),
			    [](const Title &t) { return t.imdbId == "tt0944947"; }
			);
			QVERIFY(it != titles.end());
			QCOMPARE(it->lastEpisode.season, 8);
			QVERIFY(!it->viewed);
		}
		const auto &notifications = storage.getNotifications();
		auto        it = std::find_if(
		    notifications.begin(),
		    notifications.end(),
		    [](const Notification &n) { return n.imdbId == "tt0944947"; }
		);
		QVERIFY(it != notifications.end());
		QCOMPARE(it->type, NotificationType::NewSeason);
	}

	void runNoNotificationWhenSeasonCountUnchanged()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		// Game of Thrones: 8 seasons, 6 episodes in the last season (final counts)
		Title t = makeSeries("tt0944947", QDate{}, 8);
		t.lastEpisode.episode = 6;
		storage.addTitle(t, QPixmap{});
		LibraryUpdate su(storage);
		su.run();
		QVERIFY(storage.getNotifications().empty());
	}

	void runWithInvalidKeyEmitsApiKeyError()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdate su(storage);
		QVERIFY(!su.isEmpty());
		QSignalSpy spy(&su, &LibraryUpdate::apiKeyError);
		su.run();
		QCOMPARE(spy.count(), 1);
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
		LibraryUpdate su(storage);
		auto          g = storage.lock();
		const auto   &overflow = storage.getUpdatePriority(g);
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
		LibraryUpdate su(storage);
		auto          g = storage.lock();
		const auto   &overflow = storage.getUpdatePriority(g);
		QCOMPARE((int)overflow.size(), 2);
		QVERIFY(
		    std::find(overflow.begin(), overflow.end(), QString("tt0000003")) ==
		    overflow.end()
		);
	}
};

#include "TestLibraryUpdate.moc"

QObject *createTestLibraryUpdate()
{
	return new TestLibraryUpdate();
}
