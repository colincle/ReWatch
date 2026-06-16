#include "MainWindow.hpp"
#include "AppMenuBar.hpp"
#include "TopBar.hpp"
#include "Spinner.hpp"
#include "ErrorCard.hpp"
#include "ErrorMessages.hpp"
#include "ColorPalette.hpp"

#include <QApplication>
#include <QLabel>
#include <QShortcut>
#include <QVBoxLayout>

static constexpr int TOPBAR_HEIGHT = 70;
static constexpr int ERROR_CARD_MARGIN = 5;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setMinimumSize(800, 600);
	resize(1200, 800);

	setupLayout();
	setupMenuBar();
	setupShortcuts();
	connectSignals();
	setupErrorCard();
	setupSeasonUpdateController();
	seasonUpdateController->start();
}

void MainWindow::setupLayout()
{
	auto *central = new QWidget(this);
	setCentralWidget(central);

	auto *layout = new QVBoxLayout(central);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	topBar = new TopBar;
	addBar = new AddBar;
	searchResults = new SearchResults(appStorage);
	libraryView = new LibraryView(appStorage);

	topBar->setFixedHeight(TOPBAR_HEIGHT);
	addBar->setFixedHeight(TOPBAR_HEIGHT);

	addBar->hide();
	searchResults->hide();

	layout->addWidget(topBar);
	layout->addWidget(addBar);
	layout->addWidget(searchResults);
	layout->addWidget(libraryView);
}

void MainWindow::setupErrorCard()
{
	errorCard = new ErrorCard(this, API_KEY_ERROR_MESSAGE);

	errorCard->move(ERROR_CARD_MARGIN, topBar->height() + ERROR_CARD_MARGIN);

	connect(&appStorage, &AppStorage::saveFailed, this, [this]()
	{
		errorCard->setMessage("Failed to save your library — check disk space and permissions.");
		errorCard->show();
	});
}

void MainWindow::setupSeasonUpdateController()
{
	seasonUpdateController = new SeasonUpdateController(appStorage, this);

	connect(seasonUpdateController, &SeasonUpdateController::updateStarted, this, [this]()
	{
		seasonOverlay = makeSeasonOverlay();
		appMenuBar->setEnabled(false);
	});

	connect(seasonUpdateController, &SeasonUpdateController::updateFinished, this, [this]()
	{
		appMenuBar->setEnabled(true);
		seasonOverlay->deleteLater();
		seasonOverlay = nullptr;
	});

	connect(seasonUpdateController, &SeasonUpdateController::updateFailed, this, [this](const QString & message)
	{
		errorCard->setMessage(message);
		errorCard->show();
	});

	connect(&appStorage, &AppStorage::apiKeyChanged, seasonUpdateController, &SeasonUpdateController::start);
}

QWidget *MainWindow::makeSeasonOverlay()
{
	auto *overlay = new QWidget(this);
	overlay->setGeometry(rect());
	overlay->setStyleSheet("background-color: " COLOR_BG_PRIMARY ";");

	auto *layout = new QVBoxLayout(overlay);

	auto *spinner = new Spinner(COLOR_ACCENT, 8, overlay);
	spinner->setFixedSize(48, 48);

	auto *label = new QLabel("Looking for new TV show seasons...", overlay);
	label->setStyleSheet("color: " COLOR_TEXT_SECONDARY "; font-size: 16px;");
	label->setAlignment(Qt::AlignCenter);

	layout->addStretch();
	layout->addWidget(spinner, 0, Qt::AlignCenter);
	layout->addSpacing(16);
	layout->addWidget(label, 0, Qt::AlignCenter);
	layout->addStretch();

	overlay->show();
	overlay->raise();
	return overlay;
}

void MainWindow::setupShortcuts()
{
	auto *quitShortcut = new QShortcut(QKeySequence::Close, this);
	connect(quitShortcut, &QShortcut::activated, qApp, &QApplication::quit);
}

void MainWindow::setupMenuBar()
{
	appMenuBar = new AppMenuBar(appStorage, this);
	setMenuBar(appMenuBar);
}

void MainWindow::enterAddMode()
{
	topBar->hide();
	addBar->show();
	searchResults->show();
	libraryView->hide();
}

void MainWindow::enterNormalMode()
{
	topBar->show();
	addBar->hide();
	searchResults->hide();
	libraryView->show();
}

void MainWindow::connectSignals()
{
	connect(topBar, &TopBar::requestAddMode, this, &MainWindow::enterAddMode);
	connect(addBar, &AddBar::requestNormalMode, this, &MainWindow::enterNormalMode);
	connect(topBar, &TopBar::requestSort, libraryView, &LibraryView::applySort);
	connect(topBar, &TopBar::requestTab, libraryView, &LibraryView::applyTab);
	connect(addBar, &AddBar::searchRequested, searchResults, &SearchResults::search);

	connect(searchResults, &SearchResults::searchError, this, [this](const QString & message)
	{
		errorCard->setMessage(message);
		errorCard->show();
	});
}
