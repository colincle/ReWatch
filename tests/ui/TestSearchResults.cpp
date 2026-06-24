#include <QCoreApplication>
#include <QDir>
#include <QScrollArea>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "SearchResults.hpp"
#include "Spinner.hpp"

class TestSearchResults : public QObject
{
	Q_OBJECT

  private:
	static void cleanStorage()
	{
		QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
		    .removeRecursively();
	}

  private slots:

	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		cleanStorage();
	}

	void cleanupTestCase() { cleanStorage(); }

	void spinnerHiddenInitially()
	{
		AppStorage    storage;
		SearchResults results(storage);
		QVERIFY(results.findChild<Spinner *>()->isHidden());
	}

	void resultsScrollAreaHiddenInitially()
	{
		AppStorage    storage;
		SearchResults results(storage);
		QVERIFY(results.findChild<QScrollArea *>()->isHidden());
	}

	void searchShowsSpinnerAndHidesResults()
	{
		AppStorage    storage;
		SearchResults results(storage);
		// search() kicks off an async OmdbSearch; the event loop is never spun here,
		// so no network reply resolves. Only the synchronous UI toggle is observed.
		results.search("anything");
		QVERIFY(!results.findChild<Spinner *>()->isHidden());
		QVERIFY(results.findChild<QScrollArea *>()->isHidden());
	}
};

#include "TestSearchResults.moc"

QObject *createTestSearchResults()
{
	return new TestSearchResults();
}
