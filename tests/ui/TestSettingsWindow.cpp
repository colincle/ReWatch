#include <QCoreApplication>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTest>

#include "AppStorage.hpp"
#include "SettingsWindow.hpp"

class TestSettingsWindow : public QObject
{
	Q_OBJECT

  private:
	static void cleanStorage()
	{
		QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
		    .removeRecursively();
	}

	static QPushButton *buttonWithText(SettingsWindow &w, const QString &text)
	{
		for(auto *b : w.findChildren<QPushButton *>())
			if(b->text() == text)
				return b;
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

	// ── window basics ────────────────────────────────────────────────────────

	void windowTitleIsSettings()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		QCOMPARE(w.windowTitle(), QString("Settings"));
	}

	void dialogIsModal()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		QVERIFY(w.isModal());
	}

	void fourSectionTabsExist()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		QVERIFY(buttonWithText(w, "Appearance"));
		QVERIFY(buttonWithText(w, "API Key"));
		QVERIFY(buttonWithText(w, "Platforms"));
		QVERIFY(buttonWithText(w, "Rankings"));
	}

	// ── section switching ────────────────────────────────────────────────────

	void firstSectionShownByDefault()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		QCOMPARE(w.findChild<QStackedWidget *>()->currentIndex(), 0);
	}

	void clickingTabSwitchesSection()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		buttonWithText(w, "Platforms")->click();
		QCOMPARE(w.findChild<QStackedWidget *>()->currentIndex(), 2);
	}

	// ── API key section ──────────────────────────────────────────────────────

	void apiKeyFieldPrefilledFromStorage()
	{
		AppStorage storage;
		storage.setOmdbApiKey("STOREDKEY");
		SettingsWindow w(storage);
		QCOMPARE(w.findChild<QLineEdit *>()->text(), QString("STOREDKEY"));
	}

	void applyButtonDisabledWhenKeyEmpty()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		QVERIFY(!buttonWithText(w, "Add Key")->isEnabled());
	}

	void applyButtonEnabledAfterTypingKey()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		QTest::keyClicks(w.findChild<QLineEdit *>(), "abc");
		QVERIFY(buttonWithText(w, "Add Key")->isEnabled());
	}

	void applyButtonDisabledWhenFieldClearedAgain()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		auto          *edit = w.findChild<QLineEdit *>();
		QTest::keyClicks(edit, "abc");
		edit->clear();
		QVERIFY(!buttonWithText(w, "Add Key")->isEnabled());
	}

	void applyingKeyStoresItAndShowsAddingState()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		auto          *edit = w.findChild<QLineEdit *>();
		QTest::keyClicks(edit, "MYKEY123");

		auto *apply = buttonWithText(w, "Add Key");
		apply->click();

		QCOMPARE(storage.getKey(), QString("MYKEY123"));
		QVERIFY(!apply->isEnabled());
		QVERIFY(apply->text().startsWith("Adding"));
	}

	void applyingWhitespaceOnlyKeyIsIgnored()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		auto          *edit = w.findChild<QLineEdit *>();
		QTest::keyClicks(edit, "   ");

		// Field is whitespace-only, so the button stays disabled and click is a no-op.
		QVERIFY(!buttonWithText(w, "Add Key")->isEnabled());
		QVERIFY(storage.getKey().isEmpty());
	}

	// ── theme switching ──────────────────────────────────────────────────────

	void switchingToLightThemeUpdatesStorage()
	{
		AppStorage     storage; // default theme is "dark"
		SettingsWindow w(storage);
		buttonWithText(w, "Light")->click();
		QCOMPARE(storage.getTheme(), QString("light"));
	}

	void clickingCurrentThemeIsNoOp()
	{
		AppStorage     storage; // default theme is "dark"
		SettingsWindow w(storage);
		buttonWithText(w, "Dark")->click();
		QCOMPARE(storage.getTheme(), QString("dark"));
	}

	// ── daily limit spinbox ──────────────────────────────────────────────────

	void dailyLimitSpinBoxDefaultValueIs500()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		auto          *spinBox = w.findChild<QSpinBox *>();
		QVERIFY(spinBox != nullptr);
		QCOMPARE(spinBox->value(), 500);
	}

	void dailyLimitSpinBoxUpdatesStorage()
	{
		AppStorage     storage;
		SettingsWindow w(storage);
		auto          *spinBox = w.findChild<QSpinBox *>();
		QVERIFY(spinBox != nullptr);
		spinBox->setValue(200);
		QCOMPARE(storage.getMaxUpdateRequests(), 200);
	}
};

#include "TestSettingsWindow.moc"

QObject *createTestSettingsWindow()
{
	return new TestSettingsWindow();
}
