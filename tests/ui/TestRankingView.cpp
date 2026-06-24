#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QLabel>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "RankingView.hpp"

class TestRankingView : public QObject
{
	Q_OBJECT

  private:
	static void cleanStorage()
	{
		QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
		    .removeRecursively();
	}

	static Title makeMovie(const QString &id, const QString &name)
	{
		Title t;
		t.title = name;
		t.imdbId = id;
		t.type = "movie";
		t.lastViewed = QDate::currentDate();
		return t;
	}

	static Title makeTvShow(const QString &id, const QString &name)
	{
		Title t;
		t.title = name;
		t.imdbId = id;
		t.type = "series";
		t.lastViewed = QDate::currentDate();
		return t;
	}

	static QLabel *progressLabel(RankingView &rv)
	{
		for(auto *l : rv.findChildren<QLabel *>())
			if(l->text().startsWith("Ranking"))
				return l;
		return nullptr;
	}

  private slots:

	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		cleanStorage();
	}

	void init() { cleanStorage(); }

	void cleanupTestCase() { cleanStorage(); }

	void finishedEmittedWhenNoUnrankedTitles()
	{
		AppStorage  storage;
		RankingView rv(storage);
		QSignalSpy  spy(&rv, &RankingView::finished);
		rv.start();
		QCOMPARE(spy.count(), 1);
	}

	void finishedEmittedAfterAllTitlesRanked()
	{
		AppStorage storage;
		storage.addTitle(makeMovie("tt0001", "Movie A"), QPixmap{});

		RankingView rv(storage);
		QSignalSpy  spy(&rv, &RankingView::finished);
		rv.start();
		QCOMPARE(spy.count(), 1);
	}

	void progressLabelShowsCurrentProgress()
	{
		AppStorage storage;
		storage.addTitle(makeMovie("tt0001", "Movie A"), QPixmap{});

		RankingView rv(storage);
		rv.start();

		auto *label = progressLabel(rv);
		QVERIFY(label);
		QCOMPARE(label->text(), QString("Ranking Movies: 1 / 1"));
	}

	void switchesToTvShowsPhaseAfterMovies()
	{
		AppStorage storage;
		storage.addTitle(makeMovie("tt0001", "Movie A"), QPixmap{});
		storage.addTitle(makeTvShow("tt0002", "Show A"), QPixmap{});

		RankingView rv(storage);
		rv.start();

		auto *label = progressLabel(rv);
		QVERIFY(label);
		QCOMPARE(label->text(), QString("Ranking TV Shows: 1 / 1"));
	}
};

#include "TestRankingView.moc"

QObject *createTestRankingView()
{
	return new TestRankingView();
}
