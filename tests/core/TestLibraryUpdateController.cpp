#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "ErrorMessages.hpp"
#include "LibraryUpdateController.hpp"
#include "Title.hpp"

class TestLibraryUpdateController : public QObject
{
	Q_OBJECT

  private:
	static QString apiKey() { return qEnvironmentVariable("OMDB_API_KEY"); }
	static bool    hasApiKey() { return !apiKey().isEmpty(); }

	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static Title makeSeries(const QString &imdbId, QDate lastChecked = QDate{})
	{
		Title t;
		t.imdbId = imdbId;
		t.title = "Test Series";
		t.type = "series";
		t.lastEpisode.season = 3;
		t.lastChecked = lastChecked;
		return t;
	}

	static Title makeMovie()
	{
		Title t;
		t.imdbId = "tt0000001";
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

	// -- start() with no eligible titles --------------------------------------

	void startWithEmptyStorageEmitsNothing()
	{
		AppStorage              storage;
		LibraryUpdateController controller(storage);
		QSignalSpy started(&controller, &LibraryUpdateController::updateStarted);
		controller.start();
		QCoreApplication::processEvents();
		QCOMPARE(started.count(), 0);
	}

	void startWithOnlyMoviesEmitsNothing()
	{
		AppStorage storage;
		storage.addTitle(makeMovie(), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy started(&controller, &LibraryUpdateController::updateStarted);
		controller.start();
		QCoreApplication::processEvents();
		QCOMPARE(started.count(), 0);
	}

	void startWithSeriesCheckedTodayEmitsNothing()
	{
		AppStorage storage;
		storage.addTitle(makeSeries("tt0944947", QDate::currentDate()), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy started(&controller, &LibraryUpdateController::updateStarted);
		controller.start();
		QCoreApplication::processEvents();
		QCOMPARE(started.count(), 0);
	}

	// -- re-entry guard -------------------------------------------------------

	void secondStartIsIgnoredWhileRunning()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy started(&controller, &LibraryUpdateController::updateStarted);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);

		controller.start(); // sets running = true, schedules runAttempt
		controller.start(); // must be ignored

		QVERIFY(finished.wait(10000));
		QCOMPARE(started.count(), 1);
	}

	// -- integration: invalid key (internet required, no valid key needed) ----

	void invalidKeyEmitsUpdateStarted()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy started(&controller, &LibraryUpdateController::updateStarted);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(10000));
		QCOMPARE(started.count(), 1);
	}

	void invalidKeyEmitsUpdateFinished()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(10000));
		QCOMPARE(finished.count(), 1);
	}

	void invalidKeyEmitsUpdateFailed()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy failed(&controller, &LibraryUpdateController::updateFailed);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(10000));
		QCOMPARE(failed.count(), 1);
	}

	void invalidKeyUpdateFailedMessageIsApiKeyError()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy failed(&controller, &LibraryUpdateController::updateFailed);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(10000));
		QVERIFY(!failed.isEmpty());
		QCOMPARE(failed.first().first().toString(), API_KEY_ERROR_MESSAGE);
	}

	void updateStartedFiresBeforeUpdateFailed()
	{
		AppStorage storage;
		storage.setOmdbApiKey("invalidkey");
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);

		QStringList order;
		connect(
		    &controller,
		    &LibraryUpdateController::updateStarted,
		    [&order]() { order << "started"; }
		);
		connect(
		    &controller,
		    &LibraryUpdateController::updateFailed,
		    [&order](const QString &) { order << "failed"; }
		);

		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(10000));

		QVERIFY(order.size() >= 2);
		QCOMPARE(order.first(), "started");
	}

	// -- integration: valid key (OMDB_API_KEY required) -----------------------

	void successfulUpdateEmitsUpdateStarted()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy started(&controller, &LibraryUpdateController::updateStarted);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(35000));
		QCOMPARE(started.count(), 1);
	}

	void successfulUpdateEmitsUpdateFinished()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(35000));
		QCOMPARE(finished.count(), 1);
	}

	void successfulUpdateDoesNotEmitUpdateFailed()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		storage.setOmdbApiKey(apiKey());
		storage.addTitle(makeSeries("tt0944947", QDate{}), QPixmap{});
		LibraryUpdateController controller(storage);
		QSignalSpy failed(&controller, &LibraryUpdateController::updateFailed);
		QSignalSpy finished(&controller, &LibraryUpdateController::updateFinished);
		controller.start();
		QVERIFY(finished.wait(35000));
		QCOMPARE(failed.count(), 0);
	}
};

#include "TestLibraryUpdateController.moc"

QObject *createTestLibraryUpdateController()
{
	return new TestLibraryUpdateController();
}
