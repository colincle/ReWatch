#include "MainWindow.hpp"
#include "TopBar.hpp"

#include <QApplication>
#include <QInputDialog>
#include <QMenuBar>
#include <QShortcut>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(800, 600);
    resize(1200, 800);

    setupLayout();
    setupMenuBar();
    setupShortcuts();
    connectSignals();
}

void MainWindow::setupLayout()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    topBar        = new TopBar;
    addBar        = new AddBar;
    searchResults = new SearchResults(appStorage);
    libraryView   = new LibraryView(appStorage);

    topBar->setFixedHeight(70);
    addBar->setFixedHeight(70);

    addBar->hide();
    searchResults->hide();

    layout->addWidget(topBar);
    layout->addWidget(addBar);
    layout->addWidget(searchResults);
    layout->addWidget(libraryView);
}

void MainWindow::setupShortcuts()
{
    auto *quitShortcut = new QShortcut(QKeySequence::Close, this);
    connect(quitShortcut, &QShortcut::activated, qApp, &QApplication::quit);
}

void MainWindow::setupMenuBar()
{
    auto *menuBar  = new QMenuBar(this);
    auto *fileMenu = menuBar->addMenu("Omdb API key");
    auto *setKey   = new QAction("Set API Key", this);

    fileMenu->addAction(setKey);
    setMenuBar(menuBar);

    connect(setKey, &QAction::triggered, this, &MainWindow::onSetApiKeyTriggered);
}

void MainWindow::onSetApiKeyTriggered()
{
    bool ok  = false;
    QString key = QInputDialog::getText(
        this,
        "OMDb API Key",
        "Enter your API key:",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (ok && !key.isEmpty())
        appStorage.setOmdbApiKey(key);
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
    connect(topBar, &TopBar::requestAddMode,    this,        &MainWindow::enterAddMode);
    connect(addBar, &AddBar::requestNormalMode, this,        &MainWindow::enterNormalMode);
    connect(topBar, &TopBar::requestSort,       libraryView, &LibraryView::setSort);
    connect(topBar, &TopBar::requestTab,        libraryView, &LibraryView::setTab);
    connect(addBar, &AddBar::searchRequested,   searchResults, &SearchResults::search);
}