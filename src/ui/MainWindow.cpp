// Orchestrates all view transitions and owns the season-update overlay.
// buildUi() is called on construction and again on every theme change to rebuild
// the full widget tree with fresh palette values.
#include "MainWindow.hpp"
#include "AppMenuBar.hpp"
#include "TitleDetailView.hpp"
#include "TopBar.hpp"
#include "Spinner.hpp"
#include "ErrorCard.hpp"
#include "ErrorMessages.hpp"
#include "Palette.hpp"

#include <QApplication>
#include <QLabel>
#include <QShortcut>
#include <QVBoxLayout>

static constexpr int TOPBAR_HEIGHT = 70;
static constexpr int ERROR_CARD_MARGIN = 5;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	setMinimumSize(800, 600);
	resize(appStorage.getWindowSize().width, appStorage.getWindowSize().height);
	setupShortcuts();
	setupMenuBar();
	buildUi();
}

void MainWindow::buildUi()
{
	Palette::setTheme(appStorage.getTheme());
	Palette::setAccent(appStorage.getAccentColor());

	delete centralWidget();
	if(errorCard)
	{
		delete errorCard;
		errorCard = nullptr;
	}

	setupLayout();
	connectSignals();
	setupErrorCard();

	if(!seasonUpdateController)
	{
		setupSeasonUpdateController();
		seasonUpdateController->start();
	}
}

void MainWindow::setupLayout()
{
	auto *central = new QWidget(this);
	setCentralWidget(central);

	auto *layout = new QVBoxLayout(central);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	topBar = new TopBar(appStorage);
	addBar = new AddBar;
	searchResults = new SearchResults(appStorage);
	libraryView = new LibraryView(appStorage);
	titleDetailView = new TitleDetailView(appStorage);

	topBar->setFixedHeight(TOPBAR_HEIGHT);
	addBar->setFixedHeight(TOPBAR_HEIGHT);

	addBar->hide();
	searchResults->hide();
	titleDetailView->hide();

	layout->addWidget(topBar);
	layout->addWidget(addBar);
	layout->addWidget(searchResults);
	layout->addWidget(libraryView);
	layout->addWidget(titleDetailView);
}

void MainWindow::setupErrorCard()
{
	errorCard = new ErrorCard(this, API_KEY_ERROR_MESSAGE);

	errorCard->move(ERROR_CARD_MARGIN, topBar->height() + ERROR_CARD_MARGIN);

	connect(
	    &appStorage,
	    &AppStorage::saveFailed,
	    this,
	    [this]()
	    {
		    errorCard->setMessage(SAVE_ERROR_MESSAGE);
		    errorCard->show();
	    }
	);
}

void MainWindow::setupSeasonUpdateController()
{
	seasonUpdateController = new SeasonUpdateController(appStorage, this);

	connect(
	    seasonUpdateController,
	    &SeasonUpdateController::updateStarted,
	    this,
	    [this]()
	    {
		    seasonOverlay = makeSeasonOverlay();
		    appMenuBar->setEnabled(false);
	    }
	);

	connect(
	    seasonUpdateController,
	    &SeasonUpdateController::updateFinished,
	    this,
	    [this]()
	    {
		    appMenuBar->setEnabled(true);
		    seasonOverlay->deleteLater();
		    seasonOverlay = nullptr;
	    }
	);

	connect(
	    seasonUpdateController,
	    &SeasonUpdateController::updateFailed,
	    this,
	    [this](const QString &message)
	    {
		    errorCard->setMessage(message);
		    errorCard->show();
	    }
	);

	connect(
	    &appStorage,
	    &AppStorage::apiKeyChanged,
	    seasonUpdateController,
	    &SeasonUpdateController::start
	);
}

QWidget *MainWindow::makeSeasonOverlay()
{
	auto *overlay = new QWidget(this);
	overlay->setGeometry(rect());
	overlay->setStyleSheet(
	    QStringLiteral("background-color: %1;").arg(Palette::bgPrimary)
	);

	auto *layout = new QVBoxLayout(overlay);

	auto *spinner = new Spinner(Palette::accent, 8, overlay);
	spinner->setFixedSize(48, 48);

	auto *label = new QLabel("Looking for new TV show seasons...", overlay);
	label->setStyleSheet(
	    QStringLiteral("color: %1; font-size: 16px;").arg(Palette::textSecondary)
	);
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

void MainWindow::onStyleChanged()
{
	Palette::setTheme(appStorage.getTheme());
	Palette::setAccent(appStorage.getAccentColor());
	topBar->refreshStyle();
	addBar->refreshStyle();
	searchResults->refreshStyle();
	libraryView->refreshStyle();
	titleDetailView->refreshStyle();
	if(rankingView)
		rankingView->refreshStyle();
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
	connect(&appStorage, &AppStorage::styleChanged, this, &MainWindow::onStyleChanged);
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
	titleDetailView->hide();
	libraryView->show();
}

void MainWindow::enterDetailMode(const Title &title)
{
	topBar->hide();
	libraryView->hide();
	titleDetailView->setTitle(title);
	titleDetailView->show();
}

void MainWindow::startRanking()
{
	if(rankingView)
		return;

	rankingView = new RankingView(appStorage, this);
	rankingView->setGeometry(rect());
	rankingView->show();
	rankingView->raise();

	connect(
	    rankingView,
	    &RankingView::finished,
	    this,
	    [this]()
	    {
		    rankingView->deleteLater();
		    rankingView = nullptr;
	    }
	);

	rankingView->start();
}

void MainWindow::connectSignals()
{
	connect(topBar, &TopBar::requestAddMode, this, &MainWindow::enterAddMode);
	connect(
	    topBar,
	    &TopBar::titleNavigationRequested,
	    this,
	    &MainWindow::enterDetailMode
	);
	connect(addBar, &AddBar::requestNormalMode, this, &MainWindow::enterNormalMode);
	connect(topBar, &TopBar::requestSort, libraryView, &LibraryView::applySort);
	connect(topBar, &TopBar::requestTab, libraryView, &LibraryView::applyTab);
	connect(addBar, &AddBar::searchRequested, searchResults, &SearchResults::search);
	connect(libraryView, &LibraryView::titleClicked, this, &MainWindow::enterDetailMode);
	connect(
	    titleDetailView,
	    &TitleDetailView::backRequested,
	    this,
	    &MainWindow::enterNormalMode
	);

	connect(
	    searchResults,
	    &SearchResults::searchError,
	    this,
	    [this](const QString &message)
	    {
		    errorCard->setMessage(message);
		    errorCard->show();
	    }
	);

	connect(topBar, &TopBar::requestRanking, this, &MainWindow::startRanking);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	QMainWindow::resizeEvent(event);
	if(seasonOverlay)
		seasonOverlay->setGeometry(rect());
	if(rankingView)
		rankingView->setGeometry(rect());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	appStorage.setWindowSize(width(), height());
	QMainWindow::closeEvent(event);
}