#include <QClipboard>
#include <QGuiApplication>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include "AddStreamingPlatformDialog.hpp"

class TestAddStreamingPlatformDialog : public QObject
{
	Q_OBJECT

  private:
	// nameEdit is the first non-readonly QLineEdit; urlEdit is the second.
	// imagePathEdit is the readonly one.
	static QLineEdit *nameEdit(AddStreamingPlatformDialog &d)
	{
		int idx = 0;
		for(auto *e : d.findChildren<QLineEdit *>())
			if(!e->isReadOnly() && idx++ == 0)
				return e;
		return nullptr;
	}

	static QLineEdit *urlEdit(AddStreamingPlatformDialog &d)
	{
		int idx = 0;
		for(auto *e : d.findChildren<QLineEdit *>())
			if(!e->isReadOnly() && idx++ == 1)
				return e;
		return nullptr;
	}

	static QLineEdit *imagePathEdit(AddStreamingPlatformDialog &d)
	{
		for(auto *e : d.findChildren<QLineEdit *>())
			if(e->isReadOnly())
				return e;
		return nullptr;
	}

	static QPushButton *addButton(AddStreamingPlatformDialog &d)
	{
		for(auto *b : d.findChildren<QPushButton *>())
			if(b->text() == "Add")
				return b;
		return nullptr;
	}

	static QPushButton *cancelButton(AddStreamingPlatformDialog &d)
	{
		for(auto *b : d.findChildren<QPushButton *>())
			if(b->text() == "Cancel")
				return b;
		return nullptr;
	}

	static QPushButton *copyButton(AddStreamingPlatformDialog &d)
	{
		for(auto *b : d.findChildren<QPushButton *>())
			if(b->text().startsWith("Copy"))
				return b;
		return nullptr;
	}

	static QLabel *errorLabel(AddStreamingPlatformDialog &d)
	{
		for(auto *l : d.findChildren<QLabel *>())
			if(l->text().contains("already exists"))
				return l;
		return nullptr;
	}

  private slots:

	// -- initial state --------------------------------------------------------

	void addButtonInitiallyDisabled()
	{
		AddStreamingPlatformDialog d({});
		QVERIFY(!addButton(d)->isEnabled());
	}

	void errorLabelInitiallyHidden()
	{
		AddStreamingPlatformDialog d({});
		QVERIFY(errorLabel(d));
		QVERIFY(errorLabel(d)->isHidden());
	}

	void imagePathEditIsReadOnly()
	{
		AddStreamingPlatformDialog d({});
		QVERIFY(imagePathEdit(d)->isReadOnly());
	}

	void nameEditHasMaxLength20()
	{
		AddStreamingPlatformDialog d({});
		QCOMPARE(nameEdit(d)->maxLength(), 20);
	}

	void windowTitleIsCorrect()
	{
		AddStreamingPlatformDialog d({});
		QCOMPARE(d.windowTitle(), QString("Add streaming platform"));
	}

	// -- updateAddButton logic ------------------------------------------------

	void addButtonEnabledWhenNameAndUrlSet()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("Netflix");
		urlEdit(d)->setText("https://netflix.com");
		QVERIFY(addButton(d)->isEnabled());
	}

	void addButtonDisabledWhenOnlyNameSet()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("Netflix");
		QVERIFY(!addButton(d)->isEnabled());
	}

	void addButtonDisabledWhenOnlyUrlSet()
	{
		AddStreamingPlatformDialog d({});
		urlEdit(d)->setText("https://netflix.com");
		QVERIFY(!addButton(d)->isEnabled());
	}

	void addButtonDisabledWhenNameIsWhitespaceOnly()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("   ");
		urlEdit(d)->setText("https://netflix.com");
		QVERIFY(!addButton(d)->isEnabled());
	}

	void addButtonDisabledWhenUrlIsWhitespaceOnly()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("Netflix");
		urlEdit(d)->setText("   ");
		QVERIFY(!addButton(d)->isEnabled());
	}

	void addButtonDisabledWhenNameMatchesExistingName()
	{
		AddStreamingPlatformDialog d({"Netflix"});
		nameEdit(d)->setText("Netflix");
		urlEdit(d)->setText("https://netflix.com");
		QVERIFY(!addButton(d)->isEnabled());
	}

	void errorLabelVisibleWhenNameMatchesExistingName()
	{
		AddStreamingPlatformDialog d({"Netflix"});
		nameEdit(d)->setText("Netflix");
		QVERIFY(!errorLabel(d)->isHidden());
	}

	void errorLabelHiddenAfterChangingFromDuplicateName()
	{
		AddStreamingPlatformDialog d({"Netflix"});
		nameEdit(d)->setText("Netflix");
		QVERIFY(!errorLabel(d)->isHidden());
		nameEdit(d)->setText("Prime");
		QVERIFY(errorLabel(d)->isHidden());
	}

	// -- dialog signals -------------------------------------------------------

	void cancelButtonEmitsRejected()
	{
		AddStreamingPlatformDialog d({});
		QSignalSpy                 spy(&d, &QDialog::rejected);
		cancelButton(d)->click();
		QCOMPARE(spy.count(), 1);
	}

	void addButtonEmitsAcceptedWhenEnabled()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("Netflix");
		urlEdit(d)->setText("https://netflix.com");
		QSignalSpy spy(&d, &QDialog::accepted);
		addButton(d)->click();
		QCOMPARE(spy.count(), 1);
	}

	// -- platform() accessor --------------------------------------------------

	void platformNameIsSimplified()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("  Netflix  ");
		QCOMPARE(d.platform().name, QString("Netflix"));
	}

	void platformUrlIsSimplified()
	{
		AddStreamingPlatformDialog d({});
		urlEdit(d)->setText("  https://netflix.com  ");
		QCOMPARE(d.platform().url, QString("https://netflix.com"));
	}

	void platformImageIsAlwaysEmpty()
	{
		AddStreamingPlatformDialog d({});
		nameEdit(d)->setText("Netflix");
		urlEdit(d)->setText("https://netflix.com");
		QVERIFY(d.platform().image.isEmpty());
	}

	// -- imagePath() accessor -------------------------------------------------

	void imagePathInitiallyEmpty()
	{
		AddStreamingPlatformDialog d({});
		QVERIFY(d.imagePath().isEmpty());
	}

	// -- copy button ----------------------------------------------------------

	void copyButtonSetsClipboardText()
	{
		AddStreamingPlatformDialog d({});
		auto                      *btn = copyButton(d);
		QVERIFY(btn);
		btn->click();
		QCOMPARE(QGuiApplication::clipboard()->text(), QString("rewatch"));
	}

	void copyButtonDisabledAfterClick()
	{
		AddStreamingPlatformDialog d({});
		auto                      *btn = copyButton(d);
		QVERIFY(btn);
		btn->click();
		QVERIFY(!btn->isEnabled());
	}
};

#include "TestAddStreamingPlatformDialog.moc"

QObject *createTestAddStreamingPlatformDialog()
{
	return new TestAddStreamingPlatformDialog();
}
