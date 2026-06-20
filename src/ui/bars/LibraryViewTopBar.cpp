#include "LibraryViewTopBar.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"

#include <QHBoxLayout>

static constexpr int SEARCH_INPUT_WIDTH = 220;

LibraryViewTopBar::LibraryViewTopBar(QWidget *parent) : QWidget(parent)
{
	setupLayout();
	connectSignals();
}

void LibraryViewTopBar::setupLayout()
{
	auto *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	showAllButton = new TextButton("All", 40, Palette::accent, Palette::surface, this);
	showToWatchButton =
	    new TextButton("To watch", 40, Palette::accent, Palette::surface, this);
	showAllButton->toggleActive();

	searchInput = new SearchBar(this);
	searchInput->hide();

	searchButton = new IconButton(
	    AssetsPaths::searchIcon,
	    40,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	closeButton = new IconButton(
	    AssetsPaths::crossIcon,
	    40,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	zoomInButton = new IconButton(
	    AssetsPaths::zoomInIcon,
	    40,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	zoomOutButton = new IconButton(
	    AssetsPaths::zoomOutIcon,
	    40,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	zoomInButton->setAutoRepeat(true);
	zoomInButton->setAutoRepeatDelay(200);
	zoomInButton->setAutoRepeatInterval(100);
	zoomOutButton->setAutoRepeat(true);
	zoomOutButton->setAutoRepeatDelay(200);
	zoomOutButton->setAutoRepeatInterval(100);
	closeButton->hide();

	layout->addWidget(showAllButton);
	layout->addWidget(showToWatchButton);
	layout->addSpacing(50);
	layout->addWidget(zoomOutButton);
	layout->addWidget(zoomInButton);
	layout->addStretch();
	layout->addWidget(searchInput);
	layout->addWidget(searchButton);
	layout->addWidget(closeButton);
}

void LibraryViewTopBar::refreshStyle()
{
	showAllButton->updateColors(Palette::accent, Palette::surface);
	showToWatchButton->updateColors(Palette::accent, Palette::surface);
	searchButton->updateColors(Palette::accent, Palette::surface);
	closeButton->updateColors(Palette::accent, Palette::surface);
	zoomInButton->updateColors(Palette::accent, Palette::surface);
	zoomOutButton->updateColors(Palette::accent, Palette::surface);
	searchInput->refreshStyle();
}

void LibraryViewTopBar::connectSignals()
{
	connect(
	    showAllButton,
	    &QPushButton::clicked,
	    this,
	    &LibraryViewTopBar::onShowAllClicked
	);
	connect(
	    showToWatchButton,
	    &QPushButton::clicked,
	    this,
	    &LibraryViewTopBar::onShowToWatchClicked
	);
	connect(searchButton, &QPushButton::clicked, this, &LibraryViewTopBar::openSearch);
	connect(closeButton, &QPushButton::clicked, this, &LibraryViewTopBar::closeSearch);
	connect(
	    searchInput,
	    &QLineEdit::returnPressed,
	    this,
	    &LibraryViewTopBar::onSearchCommitted
	);
	connect(
	    searchInput,
	    &QLineEdit::textChanged,
	    this,
	    &LibraryViewTopBar::onSearchTextChanged
	);
	connect(
	    searchInput,
	    &SearchBar::escapePressed,
	    this,
	    &LibraryViewTopBar::closeSearch
	);
	connect(
	    zoomInButton,
	    &QPushButton::clicked,
	    this,
	    [this]() { emit zoomRequested(10); }
	);
	connect(
	    zoomOutButton,
	    &QPushButton::clicked,
	    this,
	    [this]() { emit zoomRequested(-10); }
	);
}

void LibraryViewTopBar::onShowAllClicked()
{
	if(showAllButton->isActive())
	{
		return;
	}

	showAllButton->toggleActive();
	showToWatchButton->toggleActive();
	emit filterChanged(ViewFilter::All);
}

void LibraryViewTopBar::onShowToWatchClicked()
{
	if(showToWatchButton->isActive())
	{
		return;
	}

	showAllButton->toggleActive();
	showToWatchButton->toggleActive();
	emit filterChanged(ViewFilter::ToWatch);
}

void LibraryViewTopBar::onSearchCommitted()
{
	emit searchRequested(searchInput->text().trimmed());
}

void LibraryViewTopBar::onSearchTextChanged(const QString &text)
{
	if(text.isEmpty())
	{
		emit searchRequested("");
	}
}

void LibraryViewTopBar::openSearch()
{
	searchInput->setFixedWidth(SEARCH_INPUT_WIDTH);
	searchInput->show();
	searchInput->setFocus();
	searchButton->hide();
	closeButton->show();
}

void LibraryViewTopBar::closeSearch()
{
	searchInput->clear();
	searchInput->hide();
	emit searchRequested("");
	closeButton->hide();
	searchButton->show();
}
