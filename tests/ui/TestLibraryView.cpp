#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "LibraryView.hpp"
#include "LibraryViewTopBar.hpp"
#include "TitleCard.hpp"

class TestLibraryView : public QObject
{
	Q_OBJECT

  private:
	static void cleanStorage()
	{
		QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
		    .removeRecursively();
	}

	static AppStorage *freshStorage()
	{
		cleanStorage();
		return new AppStorage();
	}

  private slots:

	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		cleanStorage();
	}

	void cleanupTestCase() { cleanStorage(); }

	void zoomWithinBoundsUpdatesCardWidth()
	{
		AppStorage storage;
		storage.setLibraryCardWidth(200);
		LibraryView lv(storage);
		QCoreApplication::processEvents();
		emit lv.findChild<LibraryViewTopBar *>()->zoomRequested(50);
		QCOMPARE(storage.getLibraryCardWidth(), 250);
	}

	void zoomAboveMaxIsIgnored()
	{
		AppStorage storage;
		storage.setLibraryCardWidth(200);
		LibraryView lv(storage);
		QCoreApplication::processEvents();
		emit lv.findChild<LibraryViewTopBar *>()->zoomRequested(200);
		QCOMPARE(storage.getLibraryCardWidth(), 200);
	}

	void zoomBelowMinIsIgnored()
	{
		AppStorage storage;
		storage.setLibraryCardWidth(200);
		LibraryView lv(storage);
		QCoreApplication::processEvents();
		emit lv.findChild<LibraryViewTopBar *>()->zoomRequested(-100);
		QCOMPARE(storage.getLibraryCardWidth(), 200);
	}

	void titleClickedEmittedWhenCardClicked()
	{
		AppStorage storage;
		Title      t;
		t.title = "Test Movie";
		t.imdbId = "tt0000001";
		t.type = "movie";
		storage.addTitle(t, QPixmap{});

		LibraryView lv(storage);
		QCoreApplication::processEvents();

		auto *card = lv.findChild<TitleCard *>();
		QVERIFY(card);

		int clickCount = 0;
		QObject::connect(
		    &lv,
		    &LibraryView::titleClicked,
		    [&](const Title &) { ++clickCount; }
		);
		emit card->clicked();
		QCOMPARE(clickCount, 1);
	}
};

#include "TestLibraryView.moc"

QObject *createTestLibraryView()
{
	return new TestLibraryView();
}
