#include "MainWindow.hpp"
#include "TopBar.hpp"

#include <QVBoxLayout>
#include <QMenuBar>
#include <QInputDialog>
#include <QShortcut>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(800, 600);
    resize(1200, 800);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    topBar       = new TopBar;
    addBar       = new AddBar;
    searchResults = new SearchResults(appStorage);

    topBar->setFixedHeight(70);
    addBar->setFixedHeight(70);

    addBar->hide();
    searchResults->hide();

    layout->addWidget(topBar);
    layout->addWidget(addBar);
    layout->addWidget(searchResults);

    auto *quitShortcut = new QShortcut(QKeySequence::Close, this);
    connect(quitShortcut, &QShortcut::activated, qApp, &QApplication::quit);

    setupMenuBar();
    connectMainWindow();
}

void MainWindow::setupMenuBar()
{
    auto *menuBar = new QMenuBar(this);
    auto *fileMenu = menuBar->addMenu("Omdb API key");
    auto *setKey = new QAction("Set API Key", this);

    fileMenu->addAction(setKey);
    setMenuBar(menuBar);

    connect(setKey, &QAction::triggered, this, &MainWindow::onSetApiKeyTriggered);
}

void MainWindow::onSetApiKeyTriggered()
{
    bool ok = false;

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

void MainWindow::connectMainWindow()
{
    connect(topBar, &TopBar::requestAddMode, this, [this]() {
        topBar->hide();
        addBar->show();
        searchResults->show();
    });

    connect(addBar, &AddBar::requestNormalMode, this, [this]() {
        topBar->show();
        addBar->hide();
        searchResults->hide();
    });

    connect(addBar, &AddBar::searchRequested, searchResults, &SearchResults::search);
}