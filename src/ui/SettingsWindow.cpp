#include "SettingsWindow.hpp"
#include "Palette.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

static QString tabActive()
{
	return QStringLiteral("QPushButton {"
	                      "    background-color: %1; color: %2;"
	                      "    border: none; border-radius: 6px; padding: 5px 16px;"
	                      "}")
	    .arg(Palette::accent, Palette::textPrimary);
}

static QString tabInactive()
{
	return QStringLiteral("QPushButton {"
	                      "    background-color: %1; color: %2;"
	                      "    border: none; border-radius: 6px; padding: 5px 16px;"
	                      "}"
	                      "QPushButton:hover { color: %3; }")
	    .arg(Palette::surface, Palette::textSecondary, Palette::textPrimary);
}

SettingsWindow::SettingsWindow(AppStorage &appStorage, QWidget *parent) : QDialog(parent), appStorage(appStorage)
{
	setWindowTitle("Settings");
	setMinimumWidth(400);
	setModal(true);
	setupUi();
}

QString SettingsWindow::buildStyleSheet() const
{
	return QStringLiteral(
	           "QDialog { background-color: %1; }"
	           "QLabel { color: %2; background: transparent; }"
	           "QLineEdit { background-color: %3; color: %2; border: 1px solid %4; border-radius: 6px; padding: 6px "
	           "10px; }"
	           "QPushButton { background-color: %5; color: %2; border: none; border-radius: 6px; padding: 6px 18px; }"
	           "QPushButton:disabled { background-color: %3; color: %6; }")
	    .arg(Palette::bgPrimary, Palette::textPrimary, Palette::surface, Palette::border, Palette::accent,
	         Palette::textSecondary);
}

void SettingsWindow::setupUi()
{
	setStyleSheet(buildStyleSheet());

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(24, 24, 24, 24);
	layout->setSpacing(20);

	layout->addWidget(makeThemeSection());
	layout->addWidget(makeSeparator());
	layout->addWidget(makeApiKeySection());
	layout->addStretch();
}

QWidget *SettingsWindow::makeThemeSection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	auto *title = new QLabel("Appearance");
	QFont titleFont;
	titleFont.setPixelSize(15);
	titleFont.setBold(true);
	title->setFont(titleFont);

	auto *tabRow = new QWidget;
	auto *tabLayout = new QHBoxLayout(tabRow);
	tabLayout->setContentsMargins(0, 0, 0, 0);
	tabLayout->setSpacing(4);

	lightTab = new QPushButton("Light");
	darkTab = new QPushButton("Dark");
	lightTab->setAutoDefault(false);
	darkTab->setAutoDefault(false);

	const bool isLight = appStorage.getTheme() == "light";
	lightTab->setStyleSheet(isLight ? tabActive() : tabInactive());
	darkTab->setStyleSheet(isLight ? tabInactive() : tabActive());

	connect(lightTab, &QPushButton::clicked, this,
	        [this]()
	        {
		        if(appStorage.getTheme() == "light")
		        {
			        return;
		        }

		        appStorage.setTheme("light");
		        emit themeChanged("light");
		        refreshStyle();
		        lightTab->setStyleSheet(tabActive());
		        darkTab->setStyleSheet(tabInactive());
	        });

	connect(darkTab, &QPushButton::clicked, this,
	        [this]()
	        {
		        if(appStorage.getTheme() == "dark")
		        {
			        return;
		        }

		        appStorage.setTheme("dark");
		        emit themeChanged("dark");
		        refreshStyle();
		        darkTab->setStyleSheet(tabActive());
		        lightTab->setStyleSheet(tabInactive());
	        });

	tabLayout->addWidget(lightTab);
	tabLayout->addWidget(darkTab);
	tabLayout->addStretch();

	layout->addWidget(title);
	layout->addWidget(tabRow);

	return container;
}

QWidget *SettingsWindow::makeApiKeySection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(8);

	auto *title = new QLabel("OMDb API Key");
	QFont titleFont;
	titleFont.setPixelSize(15);
	titleFont.setBold(true);
	title->setFont(titleFont);

	apiKeyEdit = new QLineEdit;
	apiKeyEdit->setPlaceholderText("Enter your OMDb API key…");
	apiKeyEdit->setText(appStorage.getKey());

	applyButton = new QPushButton("Add Key");
	applyButton->setAutoDefault(false);
	applyButton->setEnabled(!appStorage.getKey().isEmpty() || !apiKeyEdit->text().simplified().isEmpty());

	auto *fieldRow = new QWidget;
	auto *fieldLayout = new QHBoxLayout(fieldRow);
	fieldLayout->setContentsMargins(0, 0, 0, 0);
	fieldLayout->setSpacing(8);
	fieldLayout->addWidget(apiKeyEdit, 1);
	fieldLayout->addWidget(applyButton);

	connect(apiKeyEdit, &QLineEdit::textChanged, this,
	        [this](const QString &text) { applyButton->setEnabled(!text.simplified().isEmpty()); });

	connect(apiKeyEdit, &QLineEdit::returnPressed, applyButton, &QPushButton::click);
	connect(applyButton, &QPushButton::clicked, this, &SettingsWindow::onApplyClicked);

	layout->addWidget(title);
	layout->addWidget(fieldRow);

	return container;
}

QFrame *SettingsWindow::makeSeparator()
{
	auto *line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::border));
	line->setFixedHeight(1);
	return line;
}

void SettingsWindow::refreshStyle()
{
	setStyleSheet(buildStyleSheet());
}

void SettingsWindow::onApplyClicked()
{
	const QString key = apiKeyEdit->text().simplified();

	if(key.isEmpty())
	{
		return;
	}

	applyButton->setEnabled(false);
	applyButton->setText("Adding…");

	appStorage.setOmdbApiKey(key);

	QTimer::singleShot(600, this,
	                   [this]()
	                   {
		                   applyButton->setEnabled(true);
		                   applyButton->setText("Add Key");
	                   });
}
