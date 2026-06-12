#include "TopBar.hpp"
#include "IconButton.hpp"
#include "TextButton.hpp"
#include "ColorPalette.hpp"
#include "AssetsPaths.hpp"

#include <QHBoxLayout>

TopBar::TopBar(QWidget *parent)
    : QWidget(parent)
{
    int buttonsHeight = 40;

    setStyleSheet(
        "background-color: " COLOR_BG_SECONDARY ";"
        "border-bottom: 1px solid " COLOR_BORDER ";"
    );
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(10);

    // LEFT GROUP (direct, no QWidget wrapper)
    MoviesButton = new TextButton("Movies", buttonsHeight, this);
    TvShowsButton = new TextButton("TV shows", buttonsHeight, this);
    MoviesButton->toggleActive();

    layout->addWidget(MoviesButton);
    layout->addWidget(TvShowsButton);

    // FLEX SPACE
    layout->addStretch();

    // RIGHT GROUP (direct, no QWidget wrapper)
    sortButton = new IconButton(SORT_ICON, buttonsHeight, COLOR_ACCENT, COLOR_SURFACE, this);
    rankButton = new IconButton(RANK_ICON, buttonsHeight, COLOR_ACCENT, COLOR_SURFACE, this);
    addButton  = new IconButton(ADD_ICON, buttonsHeight, COLOR_ACCENT, COLOR_SURFACE, this);

    layout->addWidget(sortButton);
    layout->addWidget(rankButton);
    layout->addWidget(addButton);

    connectButtons();
}

void TopBar::connectButtons()
{
    connect(MoviesButton, &QPushButton::clicked, this, [this]() {
        MoviesButton->toggleActive();
        TvShowsButton->toggleActive();
    });

    connect(TvShowsButton, &QPushButton::clicked, this, [this]() {
        MoviesButton->toggleActive();
        TvShowsButton->toggleActive();
    });
    
    connect(sortButton, &QPushButton::clicked, this, [this]() {
        //behavior here
    });

    connect(rankButton, &QPushButton::clicked, this, [this]() {
        //behavior here
    });

    connect(addButton, &QPushButton::clicked, this, [this]() {
        emit requestAddMode();
    });
}
