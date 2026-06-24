#include <QCoreApplication>
#include <QDir>
#include <QMenu>
#include <QStandardPaths>
#include <QTest>

#include "AppMenuBar.hpp"
#include "AppStorage.hpp"

class TestAppMenuBar : public QObject
{
	Q_OBJECT

  private:
	static QString testDataDir()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static QMenu *findMenu(AppMenuBar &bar, const QString &title)
	{
		for(QAction *a : bar.actions())
			if(a->text() == title && a->menu())
				return a->menu();
		return nullptr;
	}

	static bool hasAction(QMenu *menu, const QString &text)
	{
		for(QAction *a : menu->actions())
			if(a->text() == text)
				return true;
		return false;
	}

  private slots:
	void initTestCase()
	{
		QCoreApplication::setApplicationName("ReWatchTests");
		QDir(testDataDir()).removeRecursively();
	}

	void cleanupTestCase() { QDir(testDataDir()).removeRecursively(); }
	void init() { QDir(testDataDir()).removeRecursively(); }

	// -- menu structure --------------------------------------------------------

	void hasLibraryMenu()
	{
		AppStorage storage;
		AppMenuBar bar(storage);
		QVERIFY(findMenu(bar, "Library") != nullptr);
	}

	void hasHelpMenu()
	{
		AppStorage storage;
		AppMenuBar bar(storage);
		QVERIFY(findMenu(bar, "Help") != nullptr);
	}

	void libraryMenuHasImportAction()
	{
		AppStorage storage;
		AppMenuBar bar(storage);
		QMenu     *menu = findMenu(bar, "Library");
		QVERIFY(menu);
		QVERIFY(hasAction(menu, "Import library"));
	}

	void libraryMenuHasExportAction()
	{
		AppStorage storage;
		AppMenuBar bar(storage);
		QMenu     *menu = findMenu(bar, "Library");
		QVERIFY(menu);
		QVERIFY(hasAction(menu, "Export library"));
	}

	void libraryMenuHasSettingsAction()
	{
		AppStorage storage;
		AppMenuBar bar(storage);
		QMenu     *menu = findMenu(bar, "Library");
		QVERIFY(menu);
		QVERIFY(hasAction(menu, "Settings"));
	}

	void helpMenuHasHowToAction()
	{
		AppStorage storage;
		AppMenuBar bar(storage);
		QMenu     *menu = findMenu(bar, "Help");
		QVERIFY(menu);
		QVERIFY(hasAction(menu, "How to use"));
	}
};

#include "TestAppMenuBar.moc"

QObject *createTestAppMenuBar()
{
	return new TestAppMenuBar();
}
