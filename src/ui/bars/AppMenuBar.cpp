#include "AppMenuBar.hpp"
#include "ImportedFileValidator.hpp"

#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

AppMenuBar::AppMenuBar(AppStorage &appStorage, QWidget *parent)
	: QMenuBar(parent)
	, appStorage(appStorage)
{
	auto *libraryMenu = addMenu("Library");
	auto *setKey = new QAction("Set API Key", this);
	auto *importAction = new QAction("Import library", this);
	auto *exportAction = new QAction("Export library", this);

	libraryMenu->addAction(setKey);
	libraryMenu->addAction(importAction);
	libraryMenu->addAction(exportAction);

	connect(setKey, &QAction::triggered, this, &AppMenuBar::onSetApiKeyTriggered);
	connect(importAction, &QAction::triggered, this, &AppMenuBar::onImportLibraryTriggered);
	connect(exportAction, &QAction::triggered, this, &AppMenuBar::onExportLibraryTriggered);
}

void AppMenuBar::onSetApiKeyTriggered()
{
	QInputDialog dialog(parentWidget());
	dialog.setWindowTitle("OMDb API Key");
	dialog.setLabelText("Enter your API key:");
	dialog.setInputMode(QInputDialog::TextInput);
	dialog.setTextValue(appStorage.getKey());

	dialog.show(); // forces internal widget creation

	auto *buttonBox = dialog.findChild<QDialogButtonBox*>();
	auto *okBtn = buttonBox ? buttonBox->button(QDialogButtonBox::Ok) : nullptr;

	if(okBtn)
	{
		auto updateOk = [&]()
		{
			okBtn->setEnabled(!dialog.textValue().simplified().isEmpty());
		};
		updateOk();
		connect(&dialog, &QInputDialog::textValueChanged, this, [&](const QString &) { updateOk(); });
	}

	if(dialog.exec() == QDialog::Accepted)
	{
		appStorage.setOmdbApiKey(dialog.textValue().simplified());
	}
}

void AppMenuBar::onImportLibraryTriggered()
{
	QString zipPath = QFileDialog::getOpenFileName(
	                      parentWidget(),
	                      "Import Library",
	                      QDir::homePath(),
	                      "MovieTracker Backup (*.zip)"
	                  );

	if(zipPath.isEmpty())
	{
		return;
	}

	QMessageBox confirm(parentWidget());
	confirm.setWindowTitle("Import Library");
	confirm.setText("This will overwrite your current library.");
	confirm.setInformativeText("This action cannot be undone. Are you sure you want to continue?");
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
		QMessageBox::warning(parentWidget(), "Import failed", "Could not extract the library.");
	}
}

void AppMenuBar::onExportLibraryTriggered()
{
	QString zipPath = QFileDialog::getSaveFileName(
	                      parentWidget(),
	                      "Export Library",
	                      QDir::homePath() + "/movieTracker_backup.zip",
	                      "MovieTracker Backup (*.zip)"
	                  );

	if(zipPath.isEmpty())
	{
		return;
	}

	if(!appStorage.exportTo(zipPath))
	{
		QMessageBox::warning(parentWidget(), "Export failed", "Could not create the backup file.");
	}
}
