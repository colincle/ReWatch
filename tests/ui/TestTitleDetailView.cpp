#include <QCoreApplication>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "HoverButton.hpp"
#include "IconButton.hpp"
#include "Title.hpp"
#include "TitleDetailView.hpp"

class TestTitleDetailView : public QObject
{
	Q_OBJECT

  private:
	static void cleanStorage()
	{
		QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
		    .removeRecursively();
	}

	static Title makeTitle(const QString &id, const QString &name)
	{
		Title t;
		t.title = name;
		t.imdbId = id;
		t.type = "movie";
		return t;
	}

	// Top-bar IconButtons in creation order: back, unrank, upload-poster, delete.
	// Back is always first, delete always last.
	static QList<IconButton *> iconButtons(TitleDetailView &v)
	{
		QList<IconButton *> result;
		for(auto *b : v.findChildren<HoverButton *>())
			if(auto *ib = dynamic_cast<IconButton *>(b))
				result.append(ib);
		return result;
	}

	static IconButton *backButton(TitleDetailView &v) { return iconButtons(v).first(); }
	static IconButton *deleteButton(TitleDetailView &v) { return iconButtons(v).last(); }
	static IconButton *unrankButton(TitleDetailView &v) { return iconButtons(v).at(1); }
	static IconButton *uploadPosterButton(TitleDetailView &v)
	{
		return iconButtons(v).at(2);
	}

	// toWatch/watched buttons use alwaysShowText, so their label is queryable.
	static QPushButton *buttonWithText(TitleDetailView &v, const QString &text)
	{
		for(auto *b : v.findChildren<QPushButton *>())
			if(b->text() == text)
				return b;
		return nullptr;
	}

	static bool hasLabel(TitleDetailView &v, const QString &text)
	{
		for(auto *l : v.findChildren<QLabel *>())
			if(l->text() == text)
				return true;
		return false;
	}

	static bool hasLabelContaining(TitleDetailView &v, const QString &needle)
	{
		for(auto *l : v.findChildren<QLabel *>())
			if(l->text().contains(needle))
				return true;
		return false;
	}

  private slots:

	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		cleanStorage();
	}

	void init() { cleanStorage(); }

	void cleanupTestCase() { cleanStorage(); }

	// ── back / delete ────────────────────────────────────────────────────────

	void backButtonEmitsBackRequested()
	{
		AppStorage      storage;
		TitleDetailView v(storage);
		QSignalSpy      spy(&v, &TitleDetailView::backRequested);
		backButton(v)->click();
		QCOMPARE(spy.count(), 1);
	}

	void deleteButtonRemovesTitleAndEmitsBack()
	{
		AppStorage storage;
		storage.addTitle(makeTitle("tt1", "X"), QPixmap{});
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1", "X"));

		QSignalSpy spy(&v, &TitleDetailView::backRequested);
		deleteButton(v)->click();

		QVERIFY(!storage.contains("tt1"));
		QCOMPARE(spy.count(), 1);
	}

	// ── title content ────────────────────────────────────────────────────────

	void setTitleDisplaysTitleName()
	{
		AppStorage      storage;
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1375666", "Inception"));
		QVERIFY(hasLabel(v, "Inception"));
	}

	void metaSectionShowsWatchedYesWhenViewed()
	{
		AppStorage storage;
		Title      t = makeTitle("tt1", "X");
		t.viewed = true;
		TitleDetailView v(storage);
		v.setTitle(t);
		QVERIFY(hasLabel(v, "Yes"));
	}

	void metaSectionShowsWatchedNoWhenNotViewed()
	{
		AppStorage      storage;
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1", "X"));
		QVERIFY(hasLabel(v, "No"));
	}

	// ── watch-state buttons ──────────────────────────────────────────────────

	void toWatchButtonVisibleWhenNotViewed()
	{
		AppStorage      storage;
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1", "X"));
		QVERIFY(!buttonWithText(v, "To watch")->isHidden());
		QVERIFY(buttonWithText(v, "Watched")->isHidden());
	}

	void watchedButtonVisibleWhenViewed()
	{
		AppStorage storage;
		Title      t = makeTitle("tt1", "X");
		t.viewed = true;
		TitleDetailView v(storage);
		v.setTitle(t);
		QVERIFY(!buttonWithText(v, "Watched")->isHidden());
		QVERIFY(buttonWithText(v, "To watch")->isHidden());
	}

	void togglingWatchUpdatesStorageAndSwapsButtons()
	{
		AppStorage storage;
		storage.addTitle(makeTitle("tt1", "X"), QPixmap{}); // stored viewed = false
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1", "X"));

		buttonWithText(v, "To watch")->click();

		for(const Title &t : storage.getTitles())
			if(t.imdbId == "tt1")
				QVERIFY(t.viewed);

		QVERIFY(buttonWithText(v, "To watch")->isHidden());
		QVERIFY(!buttonWithText(v, "Watched")->isHidden());
	}

	// ── upload-poster / unrank buttons ───────────────────────────────────────

	void uploadPosterButtonShownOnlyWhenPosterMissing()
	{
		AppStorage      storage;
		TitleDetailView v(storage);

		Title present = makeTitle("tt1", "X");
		present.posterNotFound = false;
		v.setTitle(present);
		QVERIFY(uploadPosterButton(v)->isHidden());

		Title missing = makeTitle("tt2", "Y");
		missing.posterNotFound = true;
		v.setTitle(missing);
		QVERIFY(!uploadPosterButton(v)->isHidden());
	}

	void unrankButtonShownOnlyWhenRanked()
	{
		AppStorage      storage;
		TitleDetailView v(storage);

		v.setTitle(makeTitle("tt1", "X")); // rank 0
		QVERIFY(unrankButton(v)->isHidden());

		Title ranked = makeTitle("tt2", "Y");
		ranked.rank = 3;
		v.setTitle(ranked);
		QVERIFY(!unrankButton(v)->isHidden());
	}

	void unrankButtonClearsRankInStorage()
	{
		AppStorage storage;
		Title      ranked = makeTitle("tt1", "X");
		ranked.rank = 2;
		storage.addTitle(ranked, QPixmap{});
		storage.insertRank("tt1", 0, "movie"); // ensure a real rank in storage

		TitleDetailView v(storage);
		v.setTitle(ranked);
		unrankButton(v)->click();

		for(const Title &t : storage.getTitles())
			if(t.imdbId == "tt1")
				QCOMPARE(t.rank, 0);
	}

	// ── watch-on section ─────────────────────────────────────────────────────

	void emptyPlatformsShowsHintMessage()
	{
		AppStorage      storage;
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1", "X"));
		QVERIFY(hasLabelContaining(v, "No custom streaming platforms"));
	}

	void withPlatformsShowsTryAllButton()
	{
		AppStorage storage;
		storage.addStreamingPlatform({"https://x/rewatch", "Netflix", ""}, "");
		TitleDetailView v(storage);
		v.setTitle(makeTitle("tt1", "X"));
		QVERIFY(buttonWithText(v, "Try all"));
	}
};

#include "TestTitleDetailView.moc"

QObject *createTestTitleDetailView()
{
	return new TestTitleDetailView();
}
