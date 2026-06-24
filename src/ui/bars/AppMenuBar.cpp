// ApplicationSpecificRole moves "Settings" to the macOS application menu automatically.
#include "AppMenuBar.hpp"
#include "HelpDialog.hpp"
#include "ImportedFileValidator.hpp"
#include "SettingsWindow.hpp"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

AppMenuBar::AppMenuBar(AppStorage &appStorage, QWidget *parent)
    : QMenuBar(parent), appStorage(appStorage)
{
	auto *libraryMenu = addMenu("Library");

	auto *importAction = new QAction("Import library", this);
	auto *exportAction = new QAction("Export library", this);

	auto *settings = new QAction("Settings", this);
	settings->setMenuRole(QAction::ApplicationSpecificRole);
	libraryMenu->addAction(settings);

	libraryMenu->addAction(importAction);
	libraryMenu->addAction(exportAction);

	connect(
	    importAction,
	    &QAction::triggered,
	    this,
	    &AppMenuBar::onImportLibraryTriggered
	);
	connect(
	    exportAction,
	    &QAction::triggered,
	    this,
	    &AppMenuBar::onExportLibraryTriggered
	);
	connect(
	    settings,
	    &QAction::triggered,
	    this,
	    [this]()
	    {
		    SettingsWindow window(this->appStorage, parentWidget());
		    window.exec();
	    }
	);

	auto *helpMenu = addMenu("Help");
	auto *howToAction = new QAction("How to use", this);
	helpMenu->addAction(howToAction);
	connect(
	    howToAction,
	    &QAction::triggered,
	    this,
	    [this]()
	    {
		    HelpDialog dialog(parentWidget());
		    dialog.exec();
	    }
	);
}

void AppMenuBar::onImportLibraryTriggered()
{
	QString zipPath = QFileDialog::getOpenFileName(
	    parentWidget(),
	    "Import Library",
	    QDir::homePath(),
	    "ReWatch Backup (*.zip)"
	);

	if(zipPath.isEmpty())
	{
		return;
	}

	QMessageBox confirm(parentWidget());
	confirm.setWindowTitle("Import Library");
	confirm.setText("This will overwrite your current library.");
	confirm.setInformativeText(
	    "This action cannot be undone. Are you sure you want to continue?"
	);
	confirm.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
	confirm.setDefaultButton(QMessageBox::Cancel);

	if(confirm.exec() != QMessageBox::Ok)
	{
		return;
	}

	ValidationResult result = ImportedFileValidator::validate(zipPath);

	if(!result.valid)
	{
		QMessageBox::warning(parentWidget(), "Invalid file", result.error);
		return;
	}

	if(!appStorage.importFrom(zipPath))
	{
		QMessageBox::warning(
		    parentWidget(),
		    "Import failed",
		    "Could not extract the library."
		);
	}
}

void AppMenuBar::onExportLibraryTriggered()
{
	QString zipPath = QFileDialog::getSaveFileName(
	    parentWidget(),
	    "Export Library",
	    QDir::homePath() + "/ReWatch_backup.zip",
	    "ReWatch Backup (*.zip)"
	);

	if(zipPath.isEmpty())
	{
		return;
	}

	if(!appStorage.exportTo(zipPath))
	{
		QMessageBox::warning(
		    parentWidget(),
		    "Export failed",
		    "Could not create the backup file."
		);
	}
}
