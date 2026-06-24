// switchSection() hides non-active sections via QSizePolicy::Ignored + adjustSize()
// rather than show/hide to keep the dialog from jumping between fixed sizes.
#include "SettingsWindow.hpp"
#include "AddStreamingPlatformDialog.hpp"
#include "IconButton.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"

#include <QColorDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSet>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

namespace
{

QString tabActive()
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1; color: %2;"
	           "    border: none; border-radius: 6px; padding: 5px 16px;"
	           "}"
	)
	    .arg(Palette::accent, Palette::surface);
}

QString tabInactive()
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1; color: %2;"
	           "    border: none; border-radius: 6px; padding: 5px 16px;"
	           "}"
	)
	    .arg(Palette::surface, Palette::accent);
}

QString accentButtonStyle(const QString &color)
{
	const QString darker = QColor(Palette::surface).darker(115).name();
	return QStringLiteral(
	           "QPushButton { background-color: %1; color: %2; border: none;"
	           "              border-radius: 6px; padding: 6px 18px; }"
	           "QPushButton:pressed { background-color: %3; color: %2; }"
	)
	    .arg(Palette::surface, color, darker);
}

} // namespace

SettingsWindow::SettingsWindow(AppStorage &appStorage, QWidget *parent)
    : StyledDialog(parent), appStorage(appStorage)
{
	setWindowTitle("Settings");
	setModal(true);
	setupUi();
	connect(&appStorage, &AppStorage::styleChanged, this, &SettingsWindow::refreshStyle);
}

QString SettingsWindow::buildStyleSheet() const
{
	return styleSheet() +
	       QStringLiteral("QFrame#separator { background-color: %1; border: none; }")
	           .arg(Palette::border);
}

void SettingsWindow::setupUi()
{
	setStyleSheet(buildStyleSheet());

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(24, 24, 24, 24);
	layout->setSpacing(16);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	auto *tabRow = new QWidget;
	auto *tabLayout = new QHBoxLayout(tabRow);
	tabLayout->setContentsMargins(0, 0, 0, 0);
	tabLayout->setSpacing(4);

	const char *labels[] = {"Appearance", "API Key", "Platforms", "Rankings"};
	for(int i = 0; i < 4; i++)
	{
		sectionTabBtns[i] = new QPushButton(labels[i]);
		sectionTabBtns[i]->setAutoDefault(false);
		tabLayout->addWidget(sectionTabBtns[i]);
		connect(
		    sectionTabBtns[i],
		    &QPushButton::clicked,
		    this,
		    [this, i]() { switchSection(i); }
		);
	}
	tabLayout->addStretch();

	sectionStack = new QStackedWidget;
	sectionStack->addWidget(makeAppearanceSection());
	sectionStack->addWidget(makeApiKeySection());
	sectionStack->addWidget(makeCustomStreamingPlatformsSection());
	sectionStack->addWidget(makeRankingSection());

	layout->addWidget(tabRow);
	layout->addWidget(sectionStack);

	switchSection(0);
}

void SettingsWindow::switchSection(int index)
{
	for(int i = 0; i < 4; i++)
	{
		sectionTabBtns[i]->setStyleSheet(i == index ? tabActive() : tabInactive());
		sectionStack->widget(i)->setSizePolicy(
		    QSizePolicy::Preferred,
		    i == index ? QSizePolicy::Preferred : QSizePolicy::Ignored
		);
	}
	sectionStack->setCurrentIndex(index);
	adjustSize();
}

QWidget *SettingsWindow::makeAppearanceSection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

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

	connect(lightTab, &QPushButton::clicked, this, [this]() { switchTheme("light"); });
	connect(darkTab, &QPushButton::clicked, this, [this]() { switchTheme("dark"); });

	tabLayout->addWidget(lightTab);
	tabLayout->addWidget(darkTab);
	tabLayout->addStretch();

	const bool isDark = appStorage.getTheme() == "dark";

	darkAccentRow = makeAccentRow(true, darkColorSwatch, resetDarkAccentButton);
	lightAccentRow = makeAccentRow(false, lightColorSwatch, resetLightAccentButton);

	darkAccentRow->setVisible(isDark);
	lightAccentRow->setVisible(!isDark);

	layout->addWidget(makeSectionTitle("Appearance"));
	layout->addWidget(tabRow);
	layout->addWidget(darkAccentRow);
	layout->addWidget(lightAccentRow);

	return container;
}

QWidget *SettingsWindow::makeApiKeySection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(8);

	auto *title = makeSectionTitle("OMDb API Key");

	apiKeyEdit = new QLineEdit;
	apiKeyEdit->setPlaceholderText("Enter your OMDb API key…");
	apiKeyEdit->setText(appStorage.getKey());

	applyButton = new QPushButton("Adding...");
	applyButton->setAutoDefault(false);
	applyButton->setFixedWidth(applyButton->sizeHint().width() + 8);
	applyButton->setText("Add Key");
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

	maxUpdateRequestsSpinBox = new QSpinBox;
	maxUpdateRequestsSpinBox->setRange(1, 10000);
	maxUpdateRequestsSpinBox->setValue(appStorage.getMaxUpdateRequests());
	maxUpdateRequestsSpinBox->setStyleSheet(
	    QStringLiteral(
	        "QSpinBox { background-color: %1; color: %2; border: 1px solid %3;"
	        "           border-radius: 6px; padding: 4px 8px; }"
	        "QSpinBox::up-button, QSpinBox::down-button { width: 0; }"
	    )
	        .arg(Palette::surface, Palette::textPrimary, Palette::border)
	);

	connect(
	    maxUpdateRequestsSpinBox,
	    &QSpinBox::valueChanged,
	    this,
	    [this](int value) { appStorage.setMaxUpdateRequests(value); }
	);

	auto *limitLabel = new QLabel("Daily title update limit");
	auto *limitHint = new QLabel(
	    "Each title check uses one OMDb API request. "
	    "Free accounts are capped at 1,000 requests per day — "
	    "set this below that to leave room for searches."
	);
	limitHint->setWordWrap(true);
	limitHint->setStyleSheet(
	    QStringLiteral("color: %1; font-size: 12px;").arg(Palette::textSecondary)
	);

	auto *limitRow = new QHBoxLayout;
	limitRow->setContentsMargins(0, 0, 0, 0);
	limitRow->setSpacing(8);
	limitRow->addWidget(limitLabel, 1);
	limitRow->addWidget(maxUpdateRequestsSpinBox);

	layout->addWidget(title);
	layout->addWidget(fieldRow);
	layout->addSpacing(4);
	layout->addWidget(makeSeparator());
	layout->addSpacing(4);
	layout->addLayout(limitRow);
	layout->addWidget(limitHint);

	return container;
}

QWidget *SettingsWindow::makeCustomStreamingPlatformsSection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	auto *title = makeSectionTitle("Custom streaming platforms");

	platformsList = new QWidget;
	new QVBoxLayout(platformsList);
	platformsList->layout()->setContentsMargins(0, 0, 0, 0);
	static_cast<QVBoxLayout *>(platformsList->layout())->setSpacing(6);

	connect(
	    &appStorage,
	    &AppStorage::streamingPlatformsChanged,
	    this,
	    &SettingsWindow::refreshPlatformsList
	);

	layout->addWidget(title);
	layout->addWidget(platformsList);

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

	if(platforms.size() < 10)
	{
		auto *row = new QWidget;
		auto *rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 2, 0, 2);
		rowLayout->setSpacing(10);
		rowLayout->addStretch();

		addPlatformButton = new IconButton(
		    AssetsPaths::addIcon,
		    30,
		    Palette::accent,
		    Palette::surface,
		    row
		);
		connect(
		    addPlatformButton,
		    &QPushButton::clicked,
		    this,
		    &SettingsWindow::onAddPlatformClicked
		);
		rowLayout->addWidget(addPlatformButton);

		layout->addWidget(row);
	}
	else
	{
		addPlatformButton = nullptr;
	}
}

QWidget *SettingsWindow::makeRankingSection()
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(12);

	auto *title = makeSectionTitle("Rankings");

	auto *buttonRow = new QWidget;
	auto *buttonLayout = new QHBoxLayout(buttonRow);
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	buttonLayout->setSpacing(8);

	auto *moviesBtn = new QPushButton("Reset Movies");
	auto *showsBtn = new QPushButton("Reset TV Shows");
	moviesBtn->setAutoDefault(false);
	showsBtn->setAutoDefault(false);

	const QString resetButtonStyle =
	    QStringLiteral(
	        "QPushButton { background-color: %1; color: white; border: none;"
	        "              border-radius: 6px; padding: 6px 18px; }"
	        "QPushButton:pressed { background-color: %2; }"
	    )
	        .arg(Palette::error, "#B91C1C");
	moviesBtn->setStyleSheet(resetButtonStyle);
	showsBtn->setStyleSheet(resetButtonStyle);

	connect(
	    moviesBtn,
	    &QPushButton::clicked,
	    this,
	    [this]() { confirmResetRankings("movie", "movie"); }
	);
	connect(
	    showsBtn,
	    &QPushButton::clicked,
	    this,
	    [this]() { confirmResetRankings("TV show", "series"); }
	);

	buttonLayout->addWidget(moviesBtn);
	buttonLayout->addWidget(showsBtn);
	buttonLayout->addStretch();

	layout->addWidget(title);
	layout->addWidget(buttonRow);

	return container;
}

QFrame *SettingsWindow::makeSeparator()
{
	auto *line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setObjectName("separator");
	line->setFixedHeight(1);
	return line;
}

QLabel *SettingsWindow::makeSectionTitle(const QString &text)
{
	auto *label = new QLabel(text);
	QFont font;
	font.setPixelSize(15);
	font.setBold(true);
	label->setFont(font);
	return label;
}

QWidget *
SettingsWindow::makeAccentRow(bool isDark, QPushButton *&swatch, QPushButton *&reset)
{
	const QString accent =
	    isDark ? appStorage.getDarkAccentColor() : appStorage.getLightAccentColor();

	swatch = new QPushButton("Change color");
	swatch->setAutoDefault(false);
	swatch->setStyleSheet(accentButtonStyle(accent));

	reset = new QPushButton("Reset");
	reset->setAutoDefault(false);
	reset->setVisible(accent != Palette::defaultAccent);

	connect(
	    swatch,
	    &QPushButton::clicked,
	    this,
	    [this, swatch, reset, isDark]()
	    {
		    const QString current = isDark ? appStorage.getDarkAccentColor()
		                                   : appStorage.getLightAccentColor();
		    const QColor  picked =
		        QColorDialog::getColor(QColor(current), this, "Choose accent color");
		    if(!picked.isValid())
			    return;
		    const QString hex = picked.name();
		    isDark ? appStorage.setDarkAccentColor(hex)
		           : appStorage.setLightAccentColor(hex);
		    swatch->setStyleSheet(accentButtonStyle(hex));
		    reset->setVisible(hex != Palette::defaultAccent);
	    }
	);

	connect(
	    reset,
	    &QPushButton::clicked,
	    this,
	    [this, swatch, reset, isDark]()
	    {
		    const QString def = Palette::defaultAccent;
		    isDark ? appStorage.setDarkAccentColor(def)
		           : appStorage.setLightAccentColor(def);
		    swatch->setStyleSheet(accentButtonStyle(def));
		    reset->hide();
	    }
	);

	auto *row = new QWidget;
	auto *rowLayout = new QHBoxLayout(row);
	rowLayout->setContentsMargins(0, 0, 0, 0);
	rowLayout->setSpacing(8);
	rowLayout->addWidget(swatch);
	rowLayout->addWidget(reset);
	rowLayout->addStretch();
	return row;
}

void SettingsWindow::onAddPlatformClicked()
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
	    { appStorage.addStreamingPlatform(dialog->platform(), dialog->imagePath()); }
	);
	dialog->open();
}

void SettingsWindow::confirmResetRankings(const QString &label, const QString &type)
{
	QMessageBox box(this);
	box.setWindowTitle("Reset Rankings");
	box.setText("This will remove all " + label + " rankings. This cannot be undone.");
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	box.setDefaultButton(QMessageBox::Cancel);
	if(box.exec() == QMessageBox::Yes)
		appStorage.resetRankings(type);
}

void SettingsWindow::switchTheme(const QString &theme)
{
	if(appStorage.getTheme() == theme)
		return;
	appStorage.setTheme(theme);
	if(darkAccentRow && lightAccentRow)
	{
		darkAccentRow->hide();
		lightAccentRow->hide();
		(theme == "dark" ? darkAccentRow : lightAccentRow)->show();
	}
}

void SettingsWindow::refreshStyle()
{
	setStyleSheet(buildStyleSheet());

	if(platformsList)
		refreshPlatformsList();

	if(sectionStack)
	{
		const int cur = sectionStack->currentIndex();
		for(int i = 0; i < 4; i++)
			sectionTabBtns[i]->setStyleSheet(i == cur ? tabActive() : tabInactive());
	}

	if(lightTab && darkTab)
	{
		const bool isLight = appStorage.getTheme() == "light";
		lightTab->setStyleSheet(isLight ? tabActive() : tabInactive());
		darkTab->setStyleSheet(isLight ? tabInactive() : tabActive());
	}

	auto refreshAccentRow =
	    [](QPushButton *swatch, QPushButton *reset, const QString &accent)
	{
		if(swatch)
			swatch->setStyleSheet(accentButtonStyle(accent));
		if(reset)
			reset->setVisible(accent != Palette::defaultAccent);
	};

	refreshAccentRow(
	    darkColorSwatch,
	    resetDarkAccentButton,
	    appStorage.getDarkAccentColor()
	);
	refreshAccentRow(
	    lightColorSwatch,
	    resetLightAccentButton,
	    appStorage.getLightAccentColor()
	);
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
