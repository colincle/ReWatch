#include <QCoreApplication>
#include <QDir>
#include <QEnterEvent>
#include <QEvent>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "ElidedLabel.hpp"
#include "HoverButton.hpp"
#include "IconButton.hpp"
#include "IconTextButton.hpp"
#include "Title.hpp"
#include "TitleCard.hpp"

class TestTitleCard : public QObject
{
	Q_OBJECT

  private:
	static constexpr int CARD_WIDTH = 160;

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

	// The two IconTextButtons render as collapsed (text() is empty until hovered),
	// so they can't be found by label. They are created in a fixed order in
	// TitleCard::setupButtons: [0] = "To watch" (viewed toggle), [1] = "Watched".
	static QList<IconTextButton *> textButtons(TitleCard &card)
	{
		QList<IconTextButton *> result;
		for(auto *b : card.findChildren<HoverButton *>())
			if(auto *itb = dynamic_cast<IconTextButton *>(b))
				result.append(itb);
		return result;
	}

	static IconTextButton *toWatchButton(TitleCard &card)
	{
		return textButtons(card).at(0);
	}
	static IconTextButton *watchedButton(TitleCard &card)
	{
		return textButtons(card).at(1);
	}

	// The three IconButtons have no text; identify them by corner position.
	static IconButton *iconButtonAt(TitleCard &card, bool right, bool bottom)
	{
		for(auto *b : card.findChildren<HoverButton *>())
		{
			auto *ib = dynamic_cast<IconButton *>(b);
			if(!ib)
				continue;
			const bool isRight = ib->x() > card.width() / 2;
			const bool isBottom = ib->y() > card.height() / 2;
			if(isRight == right && isBottom == bottom)
				return ib;
		}
		return nullptr;
	}

	static void sendEnter(TitleCard &card)
	{
		QEnterEvent e(QPointF(1, 1), QPointF(1, 1), QPointF(1, 1));
		QCoreApplication::sendEvent(&card, &e);
	}

	static void sendLeave(TitleCard &card)
	{
		QEvent e(QEvent::Leave);
		QCoreApplication::sendEvent(&card, &e);
	}

  private slots:

	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		cleanStorage();
	}

	void init() { cleanStorage(); }

	void cleanupTestCase() { cleanStorage(); }

	// ── geometry & content ───────────────────────────────────────────────────

	void dimensionsDerivedFromCardWidth()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		QCOMPARE(card.width(), CARD_WIDTH);
		QCOMPARE(card.height(), CARD_WIDTH * 3 / 2 + CARD_WIDTH / 5);
	}

	void titleLabelShowsTitleText()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		auto      *label = card.findChild<ElidedLabel *>();
		QVERIFY(label);
		QCOMPARE(label->text(), QString("Up"));
	}

	// ── upload-poster button visibility ──────────────────────────────────────

	void uploadButtonHiddenWhenPosterPresent()
	{
		AppStorage storage;
		Title      t = makeTitle("tt0001", "Up");
		t.posterNotFound = false;
		TitleCard card(t, storage, CARD_WIDTH);
		QVERIFY(iconButtonAt(card, true, false)->isHidden());
	}

	void uploadButtonShownWhenPosterMissing()
	{
		AppStorage storage;
		Title      t = makeTitle("tt0001", "Up");
		t.posterNotFound = true;
		TitleCard card(t, storage, CARD_WIDTH);
		QVERIFY(!iconButtonAt(card, true, false)->isHidden());
	}

	// ── clicked signal ───────────────────────────────────────────────────────

	void leftClickEmitsClicked()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		QSignalSpy spy(&card, &TitleCard::clicked);
		QTest::mouseClick(&card, Qt::LeftButton);
		QCOMPARE(spy.count(), 1);
	}

	void rightClickDoesNotEmitClicked()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		QSignalSpy spy(&card, &TitleCard::clicked);
		QTest::mouseClick(&card, Qt::RightButton);
		QCOMPARE(spy.count(), 0);
	}

	// ── hover reveals / conceals action buttons ──────────────────────────────

	void buttonsHiddenBeforeHover()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		QVERIFY(iconButtonAt(card, true, true)->isHidden()); // delete
		QVERIFY(toWatchButton(card)->isHidden());            // viewed toggle
	}

	void hoverShowsDeleteAndToWatchButtons()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		sendEnter(card);
		QVERIFY(!iconButtonAt(card, true, true)->isHidden());
		QVERIFY(!toWatchButton(card)->isHidden());
	}

	void hoverShowsWatchedButtonWhenTitleViewed()
	{
		AppStorage storage;
		Title      t = makeTitle("tt0001", "Up");
		t.viewed = true;
		TitleCard card(t, storage, CARD_WIDTH);
		sendEnter(card);
		QVERIFY(!watchedButton(card)->isHidden());
		QVERIFY(toWatchButton(card)->isHidden());
	}

	void unrankButtonHiddenOnHoverWhenUnranked()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		sendEnter(card);
		QVERIFY(iconButtonAt(card, false, false)->isHidden());
	}

	void unrankButtonShownOnHoverWhenRanked()
	{
		AppStorage storage;
		Title      t = makeTitle("tt0001", "Up");
		t.rank = 3;
		TitleCard card(t, storage, CARD_WIDTH);
		sendEnter(card);
		QVERIFY(!iconButtonAt(card, false, false)->isHidden());
	}

	void leaveHidesButtons()
	{
		AppStorage storage;
		TitleCard  card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);
		sendEnter(card);
		sendLeave(card);
		QVERIFY(iconButtonAt(card, true, true)->isHidden());
		QVERIFY(toWatchButton(card)->isHidden());
	}

	// ── button actions mutate storage ────────────────────────────────────────

	void deleteButtonRemovesTitleFromStorage()
	{
		AppStorage storage;
		storage.addTitle(makeTitle("tt0001", "Up"), QPixmap{});
		TitleCard card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);

		iconButtonAt(card, true, true)->click();
		QVERIFY(!storage.contains("tt0001"));
	}

	void toWatchButtonMarksTitleViewed()
	{
		AppStorage storage;
		storage.addTitle(makeTitle("tt0001", "Up"), QPixmap{});
		TitleCard card(makeTitle("tt0001", "Up"), storage, CARD_WIDTH);

		toWatchButton(card)->click();

		for(const Title &t : storage.getTitles())
			if(t.imdbId == "tt0001")
				QVERIFY(t.viewed);
	}

	void watchedButtonMarksTitleNotViewed()
	{
		AppStorage storage;
		Title      stored = makeTitle("tt0001", "Up");
		storage.addTitle(stored, QPixmap{});
		storage.toggleViewed("tt0001"); // addTitle forces viewed=false; flip to true
		stored.viewed = true;
		TitleCard card(stored, storage, CARD_WIDTH);

		watchedButton(card)->click();

		for(const Title &t : storage.getTitles())
			if(t.imdbId == "tt0001")
				QVERIFY(!t.viewed);
	}

	void unrankButtonClearsRankInStorage()
	{
		AppStorage storage;
		Title      stored = makeTitle("tt0001", "Up");
		stored.rank = 5;
		storage.addTitle(stored, QPixmap{});
		TitleCard card(stored, storage, CARD_WIDTH);

		iconButtonAt(card, false, false)->click();

		for(const Title &t : storage.getTitles())
			if(t.imdbId == "tt0001")
				QCOMPARE(t.rank, 0);
	}
};

#include "TestTitleCard.moc"

QObject *createTestTitleCard()
{
	return new TestTitleCard();
}
