#include "SettingsWindow.hpp"
#include "AddStreamingPlatformDialog.hpp"
#include "IconButton.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QSet>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

static QString tabActive()
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1; color: %2;"
	           "    border: none; border-radius: 6px; padding: 5px 16px;"
	           "}"
	)
	    .arg(Palette::accent, Palette::textPrimary);
}

static QString tabInactive()
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1; color: %2;"
	           "    border: none; border-radius: 6px; padding: 5px 16px;"
	           "}"
	           "QPushButton:hover { color: %3; }"
	)
	    .arg(Palette::surface, Palette::textSecondary, Palette::textPrimary);
}

SettingsWindow::SettingsWindow(AppStorage &appStorage, QWidget *parent)
    : QDialog(parent), appStorage(appStorage)
{
	setWindowTitle("Settings");
	setModal(true);
	setupUi();
}

QString SettingsWindow::buildStyleSheet() const
{
	return QStringLiteral(
	           "QDialog { background-color: %1; }"
	           "QLabel { color: %2; background: transparent; }"
	           "QLineEdit { background-color: %3; color: %2; border: 1px solid %4; "
	           "border-radius: 6px; padding: 6px "
	           "10px; }"
	           "QPushButton { background-color: %5; color: %2; border: none; "
	           "border-radius: 6px; padding: 6px 18px; }"
	           "QPushButton:disabled { background-color: %3; color: %6; }"
	)
	    .arg(
	        Palette::bgPrimary,
	        Palette::textPrimary,
	        Palette::surface,
	        Palette::border,
	        Palette::accent,
	        Palette::textSecondary
	    );
}

void SettingsWindow::setupUi()
{
	setStyleSheet(buildStyleSheet());

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(24, 24, 24, 24);
	layout->setSpacing(20);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	layout->addWidget(makeThemeSection());
	layout->addWidget(makeSeparator());
	layout->addWidget(makeApiKeySection());
	layout->addWidget(makeSeparator());
	if(auto *s = makeCustomStreamingPlatformsSection())
	{
		layout->addWidget(s);
	}
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

	connect(
	    lightTab,
	    &QPushButton::clicked,
	    this,
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
	    }
	);

	connect(
	    darkTab,
	    &QPushButton::clicked,
	    this,
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
	    }
	);

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
	applyButton->setEnabled(
	    !appStorage.getKey().isEmpty() || !apiKeyEdit->text().simplified().isEmpty()
	);

	auto *fieldRow = new QWidget;
	auto *fieldLayout = new QHBoxLayout(fieldRow);
	fieldLayout->setContentsMargins(0, 0, 0, 0);
	fieldLayout->setSpacing(8);
	fieldLayout->addWidget(apiKeyEdit, 1);
	fieldLayout->addWidget(applyButton);

	connect(
	    apiKeyEdit,
	    &QLineEdit::textChanged,
	    this,
	    [this](const QString &text)
	    { applyButton->setEnabled(!text.simplified().isEmpty()); }
	);

	connect(apiKeyEdit, &QLineEdit::returnPressed, applyButton, &QPushButton::click);
	connect(applyButton, &QPushButton::clicked, this, &SettingsWindow::onApplyClicked);

	layout->addWidget(title);
	layout->addWidget(fieldRow);

	return container;
}

QWidget *SettingsWindow::makeCustomStreamingPlatformsSection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	auto *title = new QLabel("Custom streaming platforms");
	QFont titleFont;
	titleFont.setPixelSize(15);
	titleFont.setBold(true);
	title->setFont(titleFont);

	platformsList = new QWidget;
	new QVBoxLayout(platformsList);
	platformsList->layout()->setContentsMargins(0, 0, 0, 0);
	static_cast<QVBoxLayout *>(platformsList->layout())->setSpacing(6);

	addPlatformButton =
	    new IconButton(AssetsPaths::addIcon, 30, Palette::accent, Palette::surface, this);

	connect(
	    addPlatformButton,
	    &QPushButton::clicked,
	    this,
	    [this]()
	    {
		    QSet<QString> names;
		    for(const auto &p : appStorage.getStreamingPlatforms())
			    names.insert(p.name);
		    auto *dialog = new AddStreamingPlatformDialog(names, this);
		    dialog->setAttribute(Qt::WA_DeleteOnClose);
		    connect(
		        dialog,
		        &QDialog::accepted,
		        this,
		        [this, dialog]()
		        {
			        appStorage.addStreamingPlatform(
			            dialog->platform(),
			            dialog->imagePath()
			        );
		        }
		    );
		    dialog->open();
	    }
	);

	connect(
	    &appStorage,
	    &AppStorage::streamingPlatformsChanged,
	    this,
	    &SettingsWindow::refreshPlatformsList
	);

	layout->addWidget(title);
	layout->addWidget(platformsList);
	layout->addWidget(addPlatformButton);

	refreshPlatformsList();

	return container;
}

void SettingsWindow::refreshPlatformsList()
{
	auto *layout = static_cast<QVBoxLayout *>(platformsList->layout());

	while(QLayoutItem *item = layout->takeAt(0))
	{
		delete item->widget();
		delete item;
	}

	const auto &platforms = appStorage.getStreamingPlatforms();

	for(const StreamingPlatform &p : platforms)
	{
		auto *row = new QWidget;
		auto *rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 2, 0, 2);
		rowLayout->setSpacing(10);

		auto *imgLabel = new QLabel;
		imgLabel->setFixedSize(30, 30);
		imgLabel->setAlignment(Qt::AlignCenter);
		imgLabel->setStyleSheet(
		    QStringLiteral("QLabel { background-color: %1; border-radius: 6px; }")
		        .arg(Palette::surface)
		);
		if(!p.image.isEmpty())
		{
			QPixmap px(p.image);
			if(!px.isNull())
				imgLabel->setPixmap(
				    px.scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
				);
		}
		rowLayout->addWidget(imgLabel);

		auto *nameLabel = new QLabel(p.name);
		rowLayout->addWidget(nameLabel, 1);

		auto *deleteBtn = new IconButton(
		    AssetsPaths::deleteIcon,
		    30,
		    Palette::accent,
		    Palette::surface,
		    row
		);
		connect(
		    deleteBtn,
		    &QPushButton::clicked,
		    this,
		    [this, name = p.name]() { appStorage.removeStreamingPlatform(name); }
		);
		rowLayout->addWidget(deleteBtn);

		layout->addWidget(row);
	}

	addPlatformButton->setVisible(platforms.size() < 10);
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

	QTimer::singleShot(
	    600,
	    this,
	    [this]()
	    {
		    applyButton->setEnabled(true);
		    applyButton->setText("Add Key");
	    }
	);
}
