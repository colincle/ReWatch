#include <QCoreApplication>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QUrlQuery>

#include "AppStorage.hpp"
#include "OmdbSearch.hpp"

class TestOmdbSearch : public QObject
{
	Q_OBJECT

  private:
	static QString apiKey() { return qEnvironmentVariable("OMDB_API_KEY"); }
	static bool    hasApiKey() { return !apiKey().isEmpty(); }

	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

  private slots:
	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		QDir(testDataDir()).removeRecursively();
	}

	void cleanupTestCase() { QDir(testDataDir()).removeRecursively(); }
	void init() { QDir(testDataDir()).removeRecursively(); }

	// -- makeUrl ---------------------------------------------------------------

	void makeUrlIsHttps()
	{
		QCOMPARE(OmdbSearch::makeUrl("key", "s", "batman").scheme(), "https");
	}

	void makeUrlHostIsOmdbapi()
	{
		QCOMPARE(OmdbSearch::makeUrl("key", "s", "batman").host(), "omdbapi.com");
	}

	void makeUrlContainsApiKey()
	{
		QUrl url = OmdbSearch::makeUrl("mykey123", "s", "batman");
		QCOMPARE(QUrlQuery(url).queryItemValue("apikey"), "mykey123");
	}

	void makeUrlSetsSearchParam()
	{
		QUrl url = OmdbSearch::makeUrl("key", "s", "inception");
		QCOMPARE(QUrlQuery(url).queryItemValue("s"), "inception");
	}

	void makeUrlSetsIdParam()
	{
		QUrl url = OmdbSearch::makeUrl("key", "i", "tt0468569");
		QCOMPARE(QUrlQuery(url).queryItemValue("i"), "tt0468569");
	}

	// -- isAuthError -----------------------------------------------------------

	void isAuthErrorDetectsApiKeyPhrase()
	{
		QVERIFY(OmdbSearch::isAuthError("Invalid API key!"));
	}

	void isAuthErrorDetectsAuthenticationPhrase()
	{
		QVERIFY(OmdbSearch::isAuthError("Authentication failed"));
	}

	void isAuthErrorIsCaseInsensitive()
	{
		QVERIFY(OmdbSearch::isAuthError("API KEY not found"));
		QVERIFY(OmdbSearch::isAuthError("AUTHENTICATION error"));
	}

	void isAuthErrorReturnsFalseForUnrelatedMessage()
	{
		QVERIFY(!OmdbSearch::isAuthError("Movie not found!"));
		QVERIFY(!OmdbSearch::isAuthError("Too many results."));
		QVERIFY(!OmdbSearch::isAuthError(""));
	}

	// -- search() integration --------------------------------------------------

	void searchWithInvalidKeyReturnsAuthError()
	{
		AppStorage storage;
		OmdbSearch searcher(storage, "Batman", "invalidkey");
		QSignalSpy spy(&searcher, &OmdbSearch::searchFinished);
		searcher.search();
		QVERIFY(spy.wait(10000));
		QCOMPARE(searcher.getResults().errorType, SearchErrorType::AuthInvalid);
	}

	void searchReturnsResultsForKnownTitle()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, "The Dark Knight", apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::searchFinished);
		searcher.search();
		QVERIFY(spy.wait(30000));
		QVERIFY(!searcher.getResults().titles.empty());
	}

	void searchErrorTypeIsNoneOnSuccess()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, "Batman", apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::searchFinished);
		searcher.search();
		QVERIFY(spy.wait(30000));
		QCOMPARE(searcher.getResults().errorType, SearchErrorType::None);
	}

	void searchResultsOnlyContainMoviesAndSeries()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, "Batman", apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::searchFinished);
		searcher.search();
		QVERIFY(spy.wait(30000));
		for(const ResultTitle &t : searcher.getResults().titles)
			QVERIFY(t.type == "movie" || t.type == "series");
	}

	void searchForUnknownTitleReturnsNotFound()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, "xqzwrtyunknownxyz12345", apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::searchFinished);
		searcher.search();
		QVERIFY(spy.wait(10000));
		QCOMPARE(searcher.getResults().errorType, SearchErrorType::NotFound);
		QVERIFY(searcher.getResults().titles.empty());
	}

	// -- fetchById() integration -----------------------------------------------

	void fetchByIdWithInvalidKeyEmitsTitleFetchFailed()
	{
		AppStorage storage;
		OmdbSearch searcher(storage, "invalidkey");
		QSignalSpy spy(&searcher, &OmdbSearch::titleFetchFailed);
		searcher.fetchById("tt0468569", QPixmap{}, true);
		QVERIFY(spy.wait(10000));
		QCOMPARE(spy.count(), 1);
	}

	void fetchByIdEmitsTitleFetched()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::titleFetched);
		searcher.fetchById("tt0468569", QPixmap{}, true); // The Dark Knight
		QVERIFY(spy.wait(10000));
		QCOMPARE(spy.count(), 1);
	}

	void fetchByIdAddsTitleToStorage()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::titleFetched);
		searcher.fetchById("tt0468569", QPixmap{}, true); // The Dark Knight
		QVERIFY(spy.wait(10000));
		QVERIFY(storage.contains("tt0468569"));
	}

	void fetchByIdTitleHasCorrectFields()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::titleFetched);
		searcher.fetchById("tt0468569", QPixmap{}, true); // The Dark Knight
		QVERIFY(spy.wait(10000));
		auto        g = storage.lock();
		const auto &titles = storage.getTitles(g);
		auto        it = std::find_if(
		    titles.begin(),
		    titles.end(),
		    [](const Title &t) { return t.imdbId == "tt0468569"; }
		);
		QVERIFY(it != titles.end());
		QCOMPARE(it->title, "The Dark Knight");
		QCOMPARE(it->year, "2008");
		QCOMPARE(it->type, "movie");
	}

	void fetchByIdEmitsTitleFetchFailedForInvalidId()
	{
		if(!hasApiKey())
			QSKIP("Set OMDB_API_KEY to run integration tests");
		AppStorage storage;
		OmdbSearch searcher(storage, apiKey());
		QSignalSpy spy(&searcher, &OmdbSearch::titleFetchFailed);
		searcher.fetchById("tt9999999", QPixmap{}, true);
		QVERIFY(spy.wait(10000));
		QCOMPARE(spy.count(), 1);
		QVERIFY(!storage.contains("tt9999999"));
	}
};

#include "TestOmdbSearch.moc"

QObject *createTestOmdbSearch()
{
	return new TestOmdbSearch();
}
