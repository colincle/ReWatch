#include "TopBar.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"
#include "IconButton.hpp"
#include "TextButton.hpp"

#include <QHBoxLayout>
#include <QMenu>

static const int BUTTON_HEIGHT = 40;

static QString sortMenuStyleSheet()
{
    return
        "QMenu {"
        "    background-color: " COLOR_BG_SECONDARY ";"
        "    border: 1px solid " COLOR_BORDER ";"
        "    border-radius: 8px;"
        "    padding: 4px;"
        "}"
        "QMenu::item {"
        "    color: " COLOR_TEXT_PRIMARY ";"
        "    padding: 8px 20px;"
        "    border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "    background-color: " COLOR_SURFACE ";"
        "}";
}

TopBar::TopBar(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(
        "background-color: " COLOR_BG_SECONDARY ";"
        "border-bottom: 1px solid " COLOR_BORDER ";"
    );
    setAttribute(Qt::WA_StyledBackground, true);

    setupLayout();
    connectButtons();
}

void TopBar::setupLayout()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(10);

    moviesButton  = new TextButton("Movies",   BUTTON_HEIGHT, this);
    tvShowsButton = new TextButton("TV shows", BUTTON_HEIGHT, this);
    moviesButton->toggleActive();

    sortButton = new IconButton(SORT_ICON, BUTTON_HEIGHT, COLOR_ACCENT, COLOR_SURFACE, this);
    rankButton = new IconButton(RANK_ICON, BUTTON_HEIGHT, COLOR_ACCENT, COLOR_SURFACE, this);
    addButton  = new IconButton(ADD_ICON,  BUTTON_HEIGHT, COLOR_ACCENT, COLOR_SURFACE, this);

    layout->addWidget(moviesButton);
    layout->addWidget(tvShowsButton);
    layout->addStretch();
    layout->addWidget(sortButton);
    layout->addWidget(rankButton);
    layout->addWidget(addButton);
}

void TopBar::onMoviesClicked()
{
    if (moviesButton->isActive())
        return;

    moviesButton->toggleActive();
    tvShowsButton->toggleActive();
    emit requestTab(LibraryTab::Movies);
}

void TopBar::onTvShowsClicked()
{
    if (tvShowsButton->isActive())
        return;

    moviesButton->toggleActive();
    tvShowsButton->toggleActive();
    emit requestTab(LibraryTab::TvShows);
}

void TopBar::onSortClicked()
{
    auto *menu = new QMenu(this);
    menu->setStyleSheet(sortMenuStyleSheet());

    connect(menu->addAction("A – Z"),        &QAction::triggered, this, [this]() { emit requestSort(SortMode::AlphaAZ);    });
    connect(menu->addAction("Release"),      &QAction::triggered, this, [this]() { emit requestSort(SortMode::Release);    });
    connect(menu->addAction("Last Viewed"),  &QAction::triggered, this, [this]() { emit requestSort(SortMode::LastViewed); });
    connect(menu->addAction("Rank"),         &QAction::triggered, this, [this]() { emit requestSort(SortMode::Rank);       });

    menu->popup(sortButton->mapToGlobal(QPoint(0, sortButton->height() + 4)));
}

void TopBar::connectButtons()
{
    connect(moviesButton,  &QPushButton::clicked, this, &TopBar::onMoviesClicked);
    connect(tvShowsButton, &QPushButton::clicked, this, &TopBar::onTvShowsClicked);
    connect(sortButton,    &QPushButton::clicked, this, &TopBar::onSortClicked);
    connect(addButton,     &QPushButton::clicked, this, [this]() { emit requestAddMode(); });
}