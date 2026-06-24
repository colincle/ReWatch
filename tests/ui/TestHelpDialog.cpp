#include <QLineEdit>
#include <QScrollArea>
#include <QTest>

#include "HelpDialog.hpp"
#include "HelpItems.hpp"

class TestHelpDialog : public QObject
{
	Q_OBJECT

  private:
	static QList<QWidget *> itemCards(HelpDialog &d)
	{
		return d.findChild<QScrollArea *>()->widget()->findChildren<QWidget *>(
		    Qt::FindDirectChildrenOnly
		);
	}

  private slots:

	void windowTitleIsCorrect()
	{
		HelpDialog d;
		QCOMPARE(d.windowTitle(), QString("How to use ReWatch"));
	}

	void itemCountMatchesHelpItems()
	{
		HelpDialog d;
		QCOMPARE(itemCards(d).count(), helpItems().count());
	}

	void searchHidesNonMatchingItems()
	{
		HelpDialog d;
		auto      *search = d.findChild<QLineEdit *>();
		QTest::keyClicks(search, "notification panel");
		int hiddenCount = 0;
		for(auto *card : itemCards(d))
			if(card->isHidden())
				++hiddenCount;
		QCOMPARE(hiddenCount, helpItems().count() - 1);
	}

	void searchShowsMatchingItem()
	{
		HelpDialog d;
		auto      *search = d.findChild<QLineEdit *>();
		QTest::keyClicks(search, "notification panel");
		QVERIFY(!itemCards(d).at(0)->isHidden());
	}

	void searchIsCaseInsensitive()
	{
		HelpDialog d;
		auto      *search = d.findChild<QLineEdit *>();
		QTest::keyClicks(search, "NOTIFICATION PANEL");
		QVERIFY(!itemCards(d).at(0)->isHidden());
	}

	void emptyQueryAfterSearchRestoresAllItems()
	{
		HelpDialog d;
		auto      *search = d.findChild<QLineEdit *>();
		QTest::keyClicks(search, "notification panel");
		search->clear();
		int hiddenCount = 0;
		for(auto *card : itemCards(d))
			if(card->isHidden())
				++hiddenCount;
		QCOMPARE(hiddenCount, 0);
	}
};

#include "TestHelpDialog.moc"

QObject *createTestHelpDialog()
{
	return new TestHelpDialog();
}
