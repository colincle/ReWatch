#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QStandardPaths>
#include <QTest>

#include "AddBar.hpp"
#include "AppStorage.hpp"
#include "LibraryView.hpp"
#include "MainWindow.hpp"
#include "RankingView.hpp"
#include "TitleDetailView.hpp"
#include "TopBar.hpp"

class TestMainWindow : public QObject
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

	void init() { cleanStorage(); }

	void cleanupTestCase() { cleanStorage(); }

	void enterAddModeHidesTopBarAndShowsAddComponents()
	{
		MainWindow mw;
		emit       mw.findChild<TopBar *>()->requestAddMode();
		QVERIFY(mw.findChild<TopBar *>()->isHidden());
		QVERIFY(!mw.findChild<AddBar *>()->isHidden());
		QVERIFY(!mw.findChild<SearchResults *>()->isHidden());
		QVERIFY(mw.findChild<LibraryView *>()->isHidden());
	}

	void enterNormalModeRestoresDefaultLayout()
	{
		MainWindow mw;
		emit       mw.findChild<TopBar *>()->requestAddMode();
		emit       mw.findChild<AddBar *>()->requestNormalMode();
		QVERIFY(!mw.findChild<TopBar *>()->isHidden());
		QVERIFY(mw.findChild<AddBar *>()->isHidden());
		QVERIFY(!mw.findChild<LibraryView *>()->isHidden());
	}

	void enterDetailModeHidesLibraryAndShowsDetail()
	{
		MainWindow mw;
		emit       mw.findChild<LibraryView *>()->titleClicked(Title{});
		QVERIFY(mw.findChild<TopBar *>()->isHidden());
		QVERIFY(mw.findChild<LibraryView *>()->isHidden());
		QVERIFY(!mw.findChild<TitleDetailView *>()->isHidden());
	}

	void startRankingGuardPreventsSecondView()
	{
		// Pre-seed two unranked, viewed movies so the ranking comparison stays open
		{
			AppStorage setup;
			Title      m1;
			m1.title = "Movie A";
			m1.imdbId = "tt0001";
			m1.type = "movie";
			m1.lastViewed = QDate::currentDate();
			setup.addTitle(m1, QPixmap{});
			Title m2;
			m2.title = "Movie B";
			m2.imdbId = "tt0002";
			m2.type = "movie";
			m2.lastViewed = QDate::currentDate();
			setup.addTitle(m2, QPixmap{});
		}

		MainWindow mw;
		auto      *topBar = mw.findChild<TopBar *>();

		emit topBar->requestRanking();
		QCOMPARE(mw.findChildren<RankingView *>().count(), 1);

		emit topBar->requestRanking();
		QCOMPARE(mw.findChildren<RankingView *>().count(), 1);
	}

	void closeEventSavesWindowSize()
	{
		{
			MainWindow mw;
			mw.resize(1200, 900);
			mw.close();
		}
		AppStorage fresh;
		QCOMPARE(fresh.getWindowSize().width, 1200);
		QCOMPARE(fresh.getWindowSize().height, 900);
	}
};

#include "TestMainWindow.moc"

QObject *createTestMainWindow()
{
	return new TestMainWindow();
}
