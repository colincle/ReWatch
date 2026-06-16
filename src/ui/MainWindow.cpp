#include "MainWindow.hpp"
#include "AppMenuBar.hpp"
#include "TopBar.hpp"
#include "SeasonUpdate.hpp"
#include "Spinner.hpp"
#include "ErrorCard.hpp"
#include "ColorPalette.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QLabel>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

static constexpr int TOPBAR_HEIGHT = 70;
static constexpr int ERROR_CARD_X_RATIO = 3;

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
	runSeasonUpdate();
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
	errorCard = new ErrorCard(this, "API key error — go to \"Library\" -> \"Set API Key\" to set your key.");

	const int x = topBar->width() / ERROR_CARD_X_RATIO;
	const int y = (topBar->height() - errorCard->height()) / 2;
	errorCard->move(x, y);

	connect(&appStorage, &AppStorage::saveFailed, this, [this]()
	{
		errorCard->setMessage("Failed to save your library — check disk space and permissions.");
		errorCard->show();
	});
}

void MainWindow::runSeasonUpdate()
{
	auto *queue = new SeasonUpdate(appStorage, this);

	if(queue->isEmpty())
	{
		return;
	}

	QTimer::singleShot(0, this, [this, queue]()
	{
		auto *overlay = makeSeasonOverlay();
		appMenuBar->setEnabled(false);

		QElapsedTimer elapsed;
		elapsed.start();

		bool hadError = false;
		connect(queue, &SeasonUpdate::apiKeyError, this, [&hadError]()
		{
			hadError = true;
		}, Qt::QueuedConnection);

		QEventLoop loop;
		auto future = QtConcurrent::run([queue]() { queue->updateSeries(); });

		QFutureWatcher<void> watcher;
		connect(&watcher, &QFutureWatcher<void>::finished, &loop, [&]()
		{
			const int remaining = 1000 - static_cast<int>(elapsed.elapsed());

			if(remaining > 0)
			{
				QTimer::singleShot(remaining, &loop, &QEventLoop::quit);
			}
			else
			{
				loop.quit();
			}
		});
		watcher.setFuture(future);
		loop.exec();

		appMenuBar->setEnabled(true);
		overlay->deleteLater();

		if(hadError)
		{
			errorCard->show();
		}
	});
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
	connect(topBar, &TopBar::requestSort, libraryView, &LibraryView::setSort);
	connect(topBar, &TopBar::requestTab, libraryView, &LibraryView::setTab);
	connect(addBar, &AddBar::searchRequested, searchResults, &SearchResults::search);
}
