#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"

class TestAppStorage : public QObject
{
	Q_OBJECT

  private slots:
	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		QDir(testDataDir()).removeRecursively();
	}

	void cleanupTestCase() { QDir(testDataDir()).removeRecursively(); }

	void init() { QDir(testDataDir()).removeRecursively(); }

	// ── contains ────────────────────────────────────────────────────────────

	void containsUnknownIdReturnsFalse()
	{
		AppStorage s;
		QVERIFY(!s.contains("tt9999999"));
	}

	void containsAfterAddTitleReturnsTrue()
	{
		AppStorage s;
		s.addTitle(makeTitle("tt0000001"), QPixmap{});
		QVERIFY(s.contains("tt0000001"));
	}

	// ── addTitle ─────────────────────────────────────────────────────────────

	void addTitlePersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001"), QPixmap{});
		}
		AppStorage reloaded;
		QCOMPARE((int)titlesOf(reloaded).size(), 1);
		QCOMPARE(titlesOf(reloaded)[0].imdbId, "tt0000001");
	}

	void addTitleSetsViewedFalse()
	{
		Title t = makeTitle("tt0000001");
		t.viewed = true;
		{
			AppStorage s;
			s.addTitle(t, QPixmap{});
		}
		AppStorage reloaded;
		QVERIFY(!titlesOf(reloaded)[0].viewed);
	}

	void addTitleDuplicateDoesNotAdd()
	{
		AppStorage s;
		s.addTitle(makeTitle("tt0000001"), QPixmap{});
		s.addTitle(makeTitle("tt0000001"), QPixmap{});
		QCOMPARE((int)titlesOf(s).size(), 1);

		AppStorage reloaded;
		QCOMPARE((int)titlesOf(reloaded).size(), 1);
	}

	void addTitleEmitsTitlesUpdated()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::titlesUpdated);
		s.addTitle(makeTitle("tt0000001"), QPixmap{});
		QCOMPARE(spy.count(), 1);
	}

	void addTitlePreservesFields()
	{
		Title t = makeTitle("tt0000001");
		t.title = "Inception";
		t.type = "movie";
		t.plot = "A thief enters dreams.";
		t.director = "Christopher Nolan";
		t.actors = "Leonardo DiCaprio";
		t.released = "16 Jul 2010";
		t.lastEpisode = {};

		{
			AppStorage s;
			s.addTitle(t, QPixmap{});
		}
		AppStorage   reloaded;
		const auto   ts = titlesOf(reloaded);
		const Title &r = ts[0];
		QCOMPARE(r.title, "Inception");
		QCOMPARE(r.plot, "A thief enters dreams.");
		QCOMPARE(r.director, "Christopher Nolan");
		QCOMPARE(r.actors, "Leonardo DiCaprio");
	}

	// ── deleteTitle ──────────────────────────────────────────────────────────

	void deleteTitlePersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001"), QPixmap{});
		}
		{
			AppStorage s;
			s.deleteTitle("tt0000001");
		}
		AppStorage reloaded;
		QCOMPARE((int)titlesOf(reloaded).size(), 0);
	}

	void deleteTitleUnknownIdDoesNotCrash()
	{
		AppStorage s;
		s.deleteTitle("tt9999999");
		QCOMPARE((int)titlesOf(s).size(), 0);
	}

	void deleteTitleEmitsTitlesUpdated()
	{
		AppStorage s;
		s.addTitle(makeTitle("tt0000001"), QPixmap{});
		QSignalSpy spy(&s, &AppStorage::titlesUpdated);
		s.deleteTitle("tt0000001");
		QCOMPARE(spy.count(), 1);
	}

	// ── toggleViewed ─────────────────────────────────────────────────────────

	void toggleViewedPersistsWatched()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001"), QPixmap{});
		}
		{
			AppStorage s;
			s.toggleViewed("tt0000001");
		}
		AppStorage reloaded;
		const auto ts = titlesOf(reloaded);
		QVERIFY(ts[0].viewed);
		QCOMPARE(ts[0].lastViewed, QDate::currentDate());
	}

	void toggleViewedPersistsUnwatched()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001"), QPixmap{});
		}
		{
			AppStorage s;
			s.toggleViewed("tt0000001");
		}
		{
			AppStorage s;
			s.toggleViewed("tt0000001");
		}
		AppStorage reloaded;
		QVERIFY(!titlesOf(reloaded)[0].viewed);
	}

	void toggleViewedUnknownIdDoesNotCrash()
	{
		AppStorage s;
		s.toggleViewed("tt9999999");
	}

	void toggleViewedEmitsTitlesUpdated()
	{
		AppStorage s;
		s.addTitle(makeTitle("tt0000001"), QPixmap{});
		QSignalSpy spy(&s, &AppStorage::titlesUpdated);
		s.toggleViewed("tt0000001");
		QCOMPARE(spy.count(), 1);
	}

	// ── insertRank ───────────────────────────────────────────────────────────

	void insertRankPersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
		}
		{
			AppStorage s;
			s.insertRank("tt0000001", 0, "movie");
		}
		AppStorage reloaded;
		QCOMPARE(titlesOf(reloaded)[0].rank, 1);
	}

	void insertRankShiftsExistingRanksPersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.addTitle(makeTitle("tt0000002", "movie"), QPixmap{});
			s.insertRank("tt0000001", 0, "movie");
			s.insertRank("tt0000002", 0, "movie");
		}
		AppStorage reloaded;
		const auto titles = titlesOf(reloaded);
		QCOMPARE(findById(titles, "tt0000001")->rank, 2);
		QCOMPARE(findById(titles, "tt0000002")->rank, 1);
	}

	void insertRankDoesNotAffectOtherTypes()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.addTitle(makeTitle("tt0000002", "series"), QPixmap{});
			s.insertRank("tt0000001", 0, "movie");
			s.insertRank("tt0000002", 0, "series");
		}
		AppStorage reloaded;
		const auto titles = titlesOf(reloaded);
		QCOMPARE(findById(titles, "tt0000001")->rank, 1);
		QCOMPARE(findById(titles, "tt0000002")->rank, 1);
	}

	// ── clearRank ────────────────────────────────────────────────────────────

	void clearRankPersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.insertRank("tt0000001", 0, "movie");
			s.clearRank("tt0000001");
		}
		AppStorage reloaded;
		QCOMPARE(titlesOf(reloaded)[0].rank, 0);
	}

	void clearRankShiftsHigherRanksPersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.addTitle(makeTitle("tt0000002", "movie"), QPixmap{});
			s.insertRank("tt0000001", 0, "movie");
			s.insertRank("tt0000002", 1, "movie");
			s.clearRank("tt0000001");
		}
		AppStorage reloaded;
		const auto titles = titlesOf(reloaded);
		QCOMPARE(findById(titles, "tt0000001")->rank, 0);
		QCOMPARE(findById(titles, "tt0000002")->rank, 1);
	}

	void clearRankAlreadyZeroDoesNothing()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.clearRank("tt0000001");
		}
		AppStorage reloaded;
		QCOMPARE(titlesOf(reloaded)[0].rank, 0);
	}

	// ── resetRankings ────────────────────────────────────────────────────────

	void resetRankingsPersists()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.addTitle(makeTitle("tt0000002", "movie"), QPixmap{});
			s.insertRank("tt0000001", 0, "movie");
			s.insertRank("tt0000002", 1, "movie");
			s.resetRankings("movie");
		}
		AppStorage reloaded;
		for(const Title &t : titlesOf(reloaded))
			QCOMPARE(t.rank, 0);
	}

	void resetRankingsDoesNotAffectOtherType()
	{
		{
			AppStorage s;
			s.addTitle(makeTitle("tt0000001", "movie"), QPixmap{});
			s.addTitle(makeTitle("tt0000002", "series"), QPixmap{});
			s.insertRank("tt0000001", 0, "movie");
			s.insertRank("tt0000002", 0, "series");
			s.resetRankings("movie");
		}
		AppStorage reloaded;
		const auto titles = titlesOf(reloaded);
		QCOMPARE(findById(titles, "tt0000001")->rank, 0);
		QCOMPARE(findById(titles, "tt0000002")->rank, 1);
	}

	// ── addNotifications ─────────────────────────────────────────────────────

	void addNotificationsPersists()
	{
		{
			AppStorage s;
			s.addNotifications({Notification{"tt0000001"}, Notification{"tt0000002"}});
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getNotifications().size(), 2);
	}

	void addNotificationsDeduplicatesPersists()
	{
		{
			AppStorage s;
			s.addNotifications({Notification{"tt0000001"}});
			s.addNotifications({Notification{"tt0000001"}});
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getNotifications().size(), 1);
	}

	void addNotificationsEmitsNotificationsAddedOnlyWhenNew()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::notificationsAdded);
		s.addNotifications({Notification{"tt0000001"}});
		QCOMPARE(spy.count(), 1);
		s.addNotifications({Notification{"tt0000001"}});
		QCOMPARE(spy.count(), 1);
	}

	// ── removeNotifications ──────────────────────────────────────────────────

	void removeNotificationsPersists()
	{
		{
			AppStorage s;
			s.addNotifications({Notification{"tt0000001"}, Notification{"tt0000002"}});
		}
		{
			AppStorage s;
			s.removeNotifications();
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getNotifications().size(), 0);
	}

	void removeNotificationsEmitsNotificationsChanged()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::notificationsChanged);
		s.removeNotifications();
		QCOMPARE(spy.count(), 1);
	}

	// ── setTheme ─────────────────────────────────────────────────────────────

	void setThemePersists()
	{
		{
			AppStorage s;
			s.setTheme("light");
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getTheme(), "light");
	}

	void setThemeEmitsStyleChanged()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::styleChanged);
		s.setTheme("light");
		QCOMPARE(spy.count(), 1);
	}

	// ── setDarkAccentColor / setLightAccentColor ─────────────────────────────

	void setDarkAccentColorPersists()
	{
		{
			AppStorage s;
			s.setDarkAccentColor("#aabbcc");
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getDarkAccentColor(), "#aabbcc");
	}

	void setLightAccentColorPersists()
	{
		{
			AppStorage s;
			s.setLightAccentColor("#112233");
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getLightAccentColor(), "#112233");
	}

	void setDarkAccentColorEmitsStyleChanged()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::styleChanged);
		s.setDarkAccentColor("#aabbcc");
		QCOMPARE(spy.count(), 1);
	}

	void setLightAccentColorEmitsStyleChanged()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::styleChanged);
		s.setLightAccentColor("#112233");
		QCOMPARE(spy.count(), 1);
	}

	// ── getAccentColor ───────────────────────────────────────────────────────

	void getAccentColorReturnsDarkAccentInDarkTheme()
	{
		AppStorage s;
		s.setTheme("dark");
		s.setDarkAccentColor("#111111");
		s.setLightAccentColor("#ffffff");
		QCOMPARE(s.getAccentColor(), "#111111");
	}

	void getAccentColorReturnsLightAccentInLightTheme()
	{
		AppStorage s;
		s.setTheme("light");
		s.setDarkAccentColor("#111111");
		s.setLightAccentColor("#ffffff");
		QCOMPARE(s.getAccentColor(), "#ffffff");
	}

	// ── setLibraryCardWidth ──────────────────────────────────────────────────

	void setLibraryCardWidthPersists()
	{
		{
			AppStorage s;
			s.setLibraryCardWidth(220);
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getLibraryCardWidth(), 220);
	}

	// ── setWindowSize ────────────────────────────────────────────────────────

	void setWindowSizePersists()
	{
		{
			AppStorage s;
			s.setWindowSize(1280, 900);
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getWindowSize().width, 1280);
		QCOMPARE(reloaded.getWindowSize().height, 900);
	}

	// ── setOmdbApiKey / getKey ───────────────────────────────────────────────

	void setOmdbApiKeyPersists()
	{
		{
			AppStorage s;
			s.setOmdbApiKey("abc123");
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getKey(), "abc123");
	}

	void setOmdbApiKeyEmitsApiKeyChanged()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::apiKeyChanged);
		s.setOmdbApiKey("abc123");
		QCOMPARE(spy.count(), 1);
	}

	// ── addStreamingPlatform ─────────────────────────────────────────────────

	void addStreamingPlatformPersists()
	{
		{
			AppStorage s;
			s.addStreamingPlatform(
			    {"https://netflix.com/search?q=rewatch", "Netflix", ""},
			    ""
			);
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getStreamingPlatforms().size(), 1);
		QCOMPARE(reloaded.getStreamingPlatforms()[0].name, "Netflix");
		QCOMPARE(
		    reloaded.getStreamingPlatforms()[0].url,
		    "https://netflix.com/search?q=rewatch"
		);
	}

	void addStreamingPlatformEmitsStreamingPlatformsChanged()
	{
		AppStorage s;
		QSignalSpy spy(&s, &AppStorage::streamingPlatformsChanged);
		s.addStreamingPlatform(
		    {"https://netflix.com/search?q=rewatch", "Netflix", ""},
		    ""
		);
		QCOMPARE(spy.count(), 1);
	}

	// ── removeStreamingPlatform ──────────────────────────────────────────────

	void removeStreamingPlatformPersists()
	{
		{
			AppStorage s;
			s.addStreamingPlatform(
			    {"https://netflix.com/search?q=rewatch", "Netflix", ""},
			    ""
			);
			s.removeStreamingPlatform("Netflix");
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getStreamingPlatforms().size(), 0);
	}

	void removeStreamingPlatformUnknownNameDoesNotCrash()
	{
		AppStorage s;
		s.removeStreamingPlatform("DoesNotExist");
		QCOMPARE((int)s.getStreamingPlatforms().size(), 0);
	}

	void removeStreamingPlatformEmitsStreamingPlatformsChanged()
	{
		AppStorage s;
		s.addStreamingPlatform(
		    {"https://netflix.com/search?q=rewatch", "Netflix", ""},
		    ""
		);
		QSignalSpy spy(&s, &AppStorage::streamingPlatformsChanged);
		s.removeStreamingPlatform("Netflix");
		QCOMPARE(spy.count(), 1);
	}

	// ── setMaxUpdateRequests ─────────────────────────────────────────────────

	void setMaxUpdateRequestsDefaultIs900()
	{
		AppStorage s;
		QCOMPARE(s.getMaxUpdateRequests(), 900);
	}

	void setMaxUpdateRequestsPersists()
	{
		{
			AppStorage s;
			s.setMaxUpdateRequests(200);
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getMaxUpdateRequests(), 200);
	}

	// ── addUpdateChecks ──────────────────────────────────────────────────────

	void addUpdateChecksTracksCountAndDate()
	{
		AppStorage s;
		s.addUpdateChecks(10);
		QCOMPARE(s.getChecksToday(), 10);
		QCOMPARE(s.getChecksDate(), QDate::currentDate());
	}

	void addUpdateChecksPersists()
	{
		{
			AppStorage s;
			s.addUpdateChecks(7);
		}
		AppStorage reloaded;
		QCOMPARE(reloaded.getChecksToday(), 7);
		QCOMPARE(reloaded.getChecksDate(), QDate::currentDate());
	}

	void addUpdateChecksAccumulates()
	{
		AppStorage s;
		s.addUpdateChecks(5);
		s.addUpdateChecks(3);
		QCOMPARE(s.getChecksToday(), 8);
	}

	// ── setUpdatePriority ────────────────────────────────────────────────────

	void setUpdatePriorityPersists()
	{
		{
			AppStorage s;
			s.setUpdatePriority({"tt0000001", "tt0000002"});
		}
		AppStorage  reloaded;
		auto        g = reloaded.lock();
		const auto &priority = reloaded.getUpdatePriority(g);
		QCOMPARE((int)priority.size(), 2);
		QCOMPARE(priority[0], QString("tt0000001"));
		QCOMPARE(priority[1], QString("tt0000002"));
	}

	// ── upcomingMovies ───────────────────────────────────────────────────────

	void addMovieWithFutureReleaseDateAddsToUpcoming()
	{
		AppStorage s;
		Title      t = makeTitle("tt0000001", "movie");
		t.released = QDate::currentDate().addDays(10).toString("dd MMM yyyy");
		s.addTitle(t, QPixmap{});
		QCOMPARE((int)s.getUpcomingMovies().size(), 1);
		QCOMPARE(s.getUpcomingMovies()[0], QString("tt0000001"));
	}

	void addMovieWithPastReleaseDateDoesNotAddToUpcoming()
	{
		AppStorage s;
		Title      t = makeTitle("tt0000001", "movie");
		t.released = QDate::currentDate().addDays(-1).toString("dd MMM yyyy");
		s.addTitle(t, QPixmap{});
		QVERIFY(s.getUpcomingMovies().empty());
	}

	void addSeriesNeverAddsToUpcoming()
	{
		AppStorage s;
		s.addTitle(makeTitle("tt0000001", "series"), QPixmap{});
		QVERIFY(s.getUpcomingMovies().empty());
	}

	void deleteTitleRemovesFromUpcoming()
	{
		AppStorage s;
		Title      t = makeTitle("tt0000001", "movie");
		t.released = QDate::currentDate().addDays(10).toString("dd MMM yyyy");
		s.addTitle(t, QPixmap{});
		QCOMPARE((int)s.getUpcomingMovies().size(), 1);
		s.deleteTitle("tt0000001");
		QVERIFY(s.getUpcomingMovies().empty());
	}

	void upcomingMoviesPersistAcrossReload()
	{
		{
			AppStorage s;
			Title      t = makeTitle("tt0000001", "movie");
			t.released = QDate::currentDate().addDays(5).toString("dd MMM yyyy");
			s.addTitle(t, QPixmap{});
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getUpcomingMovies().size(), 1);
		QCOMPARE(reloaded.getUpcomingMovies()[0], QString("tt0000001"));
	}

	void removeFromUpcomingMoviesPersists()
	{
		{
			AppStorage s;
			Title      t1 = makeTitle("tt0000001", "movie");
			Title      t2 = makeTitle("tt0000002", "movie");
			t1.released = QDate::currentDate().addDays(5).toString("dd MMM yyyy");
			t2.released = QDate::currentDate().addDays(5).toString("dd MMM yyyy");
			s.addTitle(t1, QPixmap{});
			s.addTitle(t2, QPixmap{});
			s.removeFromUpcomingMovies({"tt0000001"});
		}
		AppStorage reloaded;
		QCOMPARE((int)reloaded.getUpcomingMovies().size(), 1);
		QCOMPARE(reloaded.getUpcomingMovies()[0], QString("tt0000002"));
	}

	// ── notification type ────────────────────────────────────────────────────

	void notificationTypePersistsAcrossReload()
	{
		{
			AppStorage s;
			s.addNotifications({
			    {QStringLiteral("tt0000001"), NotificationType::NewSeason},
			    {QStringLiteral("tt0000002"), NotificationType::NewEpisode},
			    {QStringLiteral("tt0000003"), NotificationType::MovieRelease},
			});
		}
		AppStorage  reloaded;
		const auto &n = reloaded.getNotifications();
		QCOMPARE((int)n.size(), 3);
		QCOMPARE(n[0].type, NotificationType::NewSeason);
		QCOMPARE(n[1].type, NotificationType::NewEpisode);
		QCOMPARE(n[2].type, NotificationType::MovieRelease);
	}

  private:
	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static Title makeTitle(const QString &imdbId, const QString &type = "movie")
	{
		Title t;
		t.imdbId = imdbId;
		t.title = "Test Title " + imdbId;
		t.type = type;
		return t;
	}

	static std::vector<Title> titlesOf(AppStorage &s)
	{
		auto g = s.lock();
		return s.getTitles(g);
	}

	static std::vector<Title>::const_iterator
	findById(const std::vector<Title> &titles, const QString &imdbId)
	{
		return std::find_if(
		    titles.begin(),
		    titles.end(),
		    [&](const Title &t) { return t.imdbId == imdbId; }
		);
	}
};

#include "TestAppStorage.moc"

QObject *createTestAppStorage()
{
	return new TestAppStorage();
}
