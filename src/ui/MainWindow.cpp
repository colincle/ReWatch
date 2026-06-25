// Orchestrates all view transitions. buildUi() is called on construction and again on
// every theme change to rebuild the full widget tree with fresh palette values.
#include "MainWindow.hpp"
#include "AppMenuBar.hpp"
#include "TitleDetailView.hpp"
#include "TopBar.hpp"
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

	if(!libraryUpdateController)
	{
		setupLibraryUpdateController();
		libraryUpdateController->start();
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

void MainWindow::setupLibraryUpdateController()
{
	libraryUpdateController = new LibraryUpdateController(appStorage, this);

	topBar->connectLibraryUpdate(*libraryUpdateController);

	connect(
	    libraryUpdateController,
	    &LibraryUpdateController::updateFailed,
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
	    libraryUpdateController,
	    &LibraryUpdateController::start
	);
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
	if(rankingView)
		rankingView->setGeometry(rect());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	appStorage.setWindowSize(width(), height());
	QMainWindow::closeEvent(event);
}