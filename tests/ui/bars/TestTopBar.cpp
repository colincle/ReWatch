#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QMenu>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "SortEnums.hpp"
#include "TextButton.hpp"
#include "Title.hpp"
#include "TopBar.hpp"

class TestTopBar : public QObject
{
	Q_OBJECT

  private:
	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static QPushButton *findTextBtn(TopBar &bar, const QString &text)
	{
		for(auto *b : bar.findChildren<QPushButton *>())
			if(dynamic_cast<TextButton *>(b) && b->text() == text)
				return b;
		return nullptr;
	}

	// IconButton creation order: notificationsButton[0], sortButton[1], rankButton[2],
	// addButton[3]
	static IconButton *iconBtn(TopBar &bar, int index)
	{
		QList<IconButton *> buttons;
		for(auto *b : bar.findChildren<HoverButton *>())
			if(auto *ib = dynamic_cast<IconButton *>(b))
				buttons.append(ib);
		return buttons.value(index);
	}

	static QWidget *notificationDot(TopBar &bar)
	{
		auto *notifBtn = iconBtn(bar, 0);
		return notifBtn ? notifBtn->findChild<QWidget *>() : nullptr;
	}

	static Title makeWatchedUnranked(const QString &imdbId)
	{
		Title t;
		t.imdbId = imdbId;
		t.title = "Test";
		t.type = "movie";
		t.rank = 0;
		t.lastViewed = QDate(2024, 1, 1);
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

	// -- tab buttons ----------------------------------------------------------

	void moviesButtonAlreadyActiveDoesNotEmitRequestTab()
	{
		AppStorage storage;
		TopBar     bar(storage);
		int        count = 0;
		connect(&bar, &TopBar::requestTab, [&](LibraryTab) { ++count; });
		findTextBtn(bar, "Movies")->click(); // already active
		QCOMPARE(count, 0);
	}

	void tvShowsButtonClickedEmitsRequestTabTvShows()
	{
		AppStorage storage;
		TopBar     bar(storage);
		LibraryTab received = LibraryTab::Movies;
		int        count = 0;
		connect(
		    &bar,
		    &TopBar::requestTab,
		    [&](LibraryTab tab)
		    {
			    received = tab;
			    ++count;
		    }
		);
		findTextBtn(bar, "TV shows")->click();
		QCOMPARE(count, 1);
		QCOMPARE(received, LibraryTab::TvShows);
	}

	void tvShowsButtonAlreadyActiveDoesNotEmitRequestTab()
	{
		AppStorage storage;
		TopBar     bar(storage);
		findTextBtn(bar, "TV shows")->click(); // activate TV shows
		int count = 0;
		connect(&bar, &TopBar::requestTab, [&](LibraryTab) { ++count; });
		findTextBtn(bar, "TV shows")->click(); // already active
		QCOMPARE(count, 0);
	}

	void moviesButtonClickedAfterTvShowsEmitsRequestTabMovies()
	{
		AppStorage storage;
		TopBar     bar(storage);
		findTextBtn(bar, "TV shows")->click();
		LibraryTab received = LibraryTab::TvShows;
		int        count = 0;
		connect(
		    &bar,
		    &TopBar::requestTab,
		    [&](LibraryTab tab)
		    {
			    received = tab;
			    ++count;
		    }
		);
		findTextBtn(bar, "Movies")->click();
		QCOMPARE(count, 1);
		QCOMPARE(received, LibraryTab::Movies);
	}

	// -- add button -----------------------------------------------------------

	void addButtonClickedEmitsRequestAddMode()
	{
		AppStorage storage;
		TopBar     bar(storage);
		QSignalSpy spy(&bar, &TopBar::requestAddMode);
		iconBtn(bar, 3)->click(); // addButton
		QCOMPARE(spy.count(), 1);
	}

	// -- rank button ----------------------------------------------------------

	void rankButtonWithUnrankedWatchedTitleEmitsRequestRanking()
	{
		AppStorage storage;
		storage.addTitle(makeWatchedUnranked("tt0000001"), QPixmap{});
		TopBar     bar(storage);
		QSignalSpy spy(&bar, &TopBar::requestRanking);
		iconBtn(bar, 2)->click(); // rankButton
		QCOMPARE(spy.count(), 1);
	}

	void rankButtonWithNoTitlesDoesNotEmitRequestRanking()
	{
		AppStorage storage; // empty
		TopBar     bar(storage);
		QSignalSpy spy(&bar, &TopBar::requestRanking);
		iconBtn(bar, 2)->click();
		QCOMPARE(spy.count(), 0);
	}

	void rankButtonWithRankedTitleDoesNotEmitRequestRanking()
	{
		AppStorage storage;
		Title      t = makeWatchedUnranked("tt0000001");
		t.rank = 1; // already ranked
		storage.addTitle(t, QPixmap{});
		TopBar     bar(storage);
		QSignalSpy spy(&bar, &TopBar::requestRanking);
		iconBtn(bar, 2)->click();
		QCOMPARE(spy.count(), 0);
	}

	void rankButtonWithNeverWatchedTitleDoesNotEmitRequestRanking()
	{
		AppStorage storage;
		Title      t;
		t.imdbId = "tt0000001";
		t.title = "Test";
		t.type = "movie";
		t.rank = 0;
		// lastViewed left invalid (never watched)
		storage.addTitle(t, QPixmap{});
		TopBar     bar(storage);
		QSignalSpy spy(&bar, &TopBar::requestRanking);
		iconBtn(bar, 2)->click();
		QCOMPARE(spy.count(), 0);
	}

	// -- sort menu ------------------------------------------------------------

	void sortMenuHasFourOptions()
	{
		AppStorage storage;
		TopBar     bar(storage);
		iconBtn(bar, 1)->click(); // sortButton
		auto *menu = bar.findChild<QMenu *>(QString{}, Qt::FindDirectChildrenOnly);
		QVERIFY(menu);
		QCOMPARE(menu->actions().size(), 4);
	}

	void sortAZEmitsRequestSortAlphaAZ()
	{
		AppStorage storage;
		TopBar     bar(storage);
		iconBtn(bar, 1)->click();
		auto *menu = bar.findChild<QMenu *>(QString{}, Qt::FindDirectChildrenOnly);
		QVERIFY(menu);
		SortMode received = SortMode::Release;
		int      count = 0;
		connect(
		    &bar,
		    &TopBar::requestSort,
		    [&](SortMode m)
		    {
			    received = m;
			    ++count;
		    }
		);
		menu->actions()[0]->trigger();
		QCOMPARE(count, 1);
		QCOMPARE(received, SortMode::AlphaAZ);
	}

	void sortReleaseEmitsRequestSortRelease()
	{
		AppStorage storage;
		TopBar     bar(storage);
		iconBtn(bar, 1)->click();
		auto *menu = bar.findChild<QMenu *>(QString{}, Qt::FindDirectChildrenOnly);
		QVERIFY(menu);
		SortMode received = SortMode::AlphaAZ;
		connect(&bar, &TopBar::requestSort, [&](SortMode m) { received = m; });
		menu->actions()[1]->trigger();
		QCOMPARE(received, SortMode::Release);
	}

	void sortWatchDateEmitsRequestSortWatchDate()
	{
		AppStorage storage;
		TopBar     bar(storage);
		iconBtn(bar, 1)->click();
		auto *menu = bar.findChild<QMenu *>(QString{}, Qt::FindDirectChildrenOnly);
		QVERIFY(menu);
		SortMode received = SortMode::AlphaAZ;
		connect(&bar, &TopBar::requestSort, [&](SortMode m) { received = m; });
		menu->actions()[2]->trigger();
		QCOMPARE(received, SortMode::WatchDate);
	}

	void sortRankEmitsRequestSortRank()
	{
		AppStorage storage;
		TopBar     bar(storage);
		iconBtn(bar, 1)->click();
		auto *menu = bar.findChild<QMenu *>(QString{}, Qt::FindDirectChildrenOnly);
		QVERIFY(menu);
		SortMode received = SortMode::AlphaAZ;
		connect(&bar, &TopBar::requestSort, [&](SortMode m) { received = m; });
		menu->actions()[3]->trigger();
		QCOMPARE(received, SortMode::Rank);
	}

	// -- notification dot -----------------------------------------------------

	void notificationDotHiddenWhenNoNotifications()
	{
		AppStorage storage;
		TopBar     bar(storage);
		bar.show();
		QCoreApplication::processEvents();
		auto *dot = notificationDot(bar);
		QVERIFY(dot);
		QVERIFY(!dot->isVisible());
	}

	void notificationDotVisibleWhenNotificationsExist()
	{
		AppStorage storage;
		storage.addNotifications({Notification{"tt0000001"}});
		TopBar bar(storage);
		bar.show();
		QCoreApplication::processEvents();
		auto *dot = notificationDot(bar);
		QVERIFY(dot);
		QVERIFY(dot->isVisible());
	}

	void addingNotificationsShowsDot()
	{
		AppStorage storage;
		TopBar     bar(storage);
		bar.show();
		QCoreApplication::processEvents();
		auto *dot = notificationDot(bar);
		QVERIFY(dot);
		QVERIFY(!dot->isVisible());
		storage.addNotifications({Notification{"tt0000001"}});
		QCoreApplication::processEvents();
		QVERIFY(dot->isVisible());
	}

	void removingNotificationsHidesDot()
	{
		AppStorage storage;
		storage.addNotifications({Notification{"tt0000001"}});
		TopBar bar(storage);
		bar.show();
		QCoreApplication::processEvents();
		auto *dot = notificationDot(bar);
		QVERIFY(dot);
		QVERIFY(dot->isVisible());
		storage.removeNotifications();
		QCoreApplication::processEvents();
		QVERIFY(!dot->isVisible());
	}

	// -- misc -----------------------------------------------------------------

	void refreshStyleDoesNotCrash()
	{
		AppStorage storage;
		TopBar     bar(storage);
		bar.refreshStyle();
	}
};

#include "TestTopBar.moc"

QObject *createTestTopBar()
{
	return new TestTopBar();
}
