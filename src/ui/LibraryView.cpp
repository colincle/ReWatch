#include "LibraryView.hpp"
#include "Palette.hpp"
#include "TitleSearch.hpp"

#include <QFrame>
#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>

LibraryView::LibraryView(AppStorage &appStorage, QWidget *parent)
    : QWidget(parent), appStorage(appStorage)
{
	setupUi();
	connectSignals();

	auto guard = appStorage.lock();
	titles = appStorage.getTitles(guard);
	QTimer::singleShot(0, this, &LibraryView::populate);
}

void LibraryView::refreshStyle()
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	libraryViewTopBar->refreshStyle();
	populate();
}

void LibraryView::setupUi()
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	setAttribute(Qt::WA_StyledBackground, true);

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, MARGIN, 0, MARGIN);
	layout->setSpacing(16);

	layout->addWidget(makeTopBarWrapper());
	layout->addWidget(makeScrollArea());

	resizeTimer = new QTimer(this);
	resizeTimer->setSingleShot(true);
	resizeTimer->setInterval(150);
}

QWidget *LibraryView::makeTopBarWrapper()
{
	libraryViewTopBar = new LibraryViewTopBar(this);

	auto *wrapper = new QWidget;
	auto *layout = new QVBoxLayout(wrapper);
	layout->setContentsMargins(MARGIN, 0, MARGIN, 0);
	layout->addWidget(libraryViewTopBar);

	return wrapper;
}

QWidget *LibraryView::makeScrollArea()
{
	cardsContainer = new QWidget;
	cardsContainer->setStyleSheet("background: transparent;");

	cardsLayout = new QGridLayout(cardsContainer);
	cardsLayout->setContentsMargins(0, 0, 0, 0);
	cardsLayout->setSpacing(CARD_SPACING);
	cardsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidget(cardsContainer);
	scrollArea->setStyleSheet(
	    "QScrollArea { background: transparent; border: none; }"
	    "QScrollArea > QWidget > QWidget { background: transparent; }"
	    "QScrollBar:vertical { width: 0px; }"
	);

	return scrollArea;
}

void LibraryView::connectSignals()
{
	connect(
	    &appStorage,
	    &AppStorage::titlesUpdated,
	    this,
	    [this]() { onSearchRequested(currentQuery); }
	);

	connect(
	    libraryViewTopBar,
	    &LibraryViewTopBar::searchRequested,
	    this,
	    &LibraryView::onSearchRequested
	);
	connect(
	    libraryViewTopBar,
	    &LibraryViewTopBar::filterChanged,
	    this,
	    &LibraryView::applyFilter
	);
	connect(
	    libraryViewTopBar,
	    &LibraryViewTopBar::zoomRequested,
	    this,
	    &LibraryView::onZoomRequested
	);
	connect(resizeTimer, &QTimer::timeout, this, &LibraryView::populate);
}

void LibraryView::onZoomRequested(int zoomValue)
{
	int newCardWidth = cardWidth + zoomValue;

	if(newCardWidth > MAX_CARD_WIDTH || newCardWidth < MIN_CARD_WIDTH)
	{
		return;
	}

	cardWidth = newCardWidth;
	appStorage.setLibraryCardWidth(cardWidth);
	populate();
}

void LibraryView::onSearchRequested(const QString &query)
{
	currentQuery = query;
	auto guard = appStorage.lock();
	titles = scoreAndRankTitles(appStorage.getTitles(guard), query);
	populate();
}

void LibraryView::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	resizeTimer->start();
}

int LibraryView::computeColumns() const
{
	int available = scrollArea->viewport()->width();
	int cols = (available + CARD_SPACING) / (cardWidth + CARD_SPACING);
	return qMax(1, cols);
}

int LibraryView::computeSpacing() const
{
	int available = scrollArea->viewport()->width();
	int cols = computeColumns();
	return (available - cols * cardWidth) / (cols + 1);
}

void LibraryView::populate()
{
	clear();

	cardWidth = appStorage.getLibraryCardWidth();
	std::vector<Title> filtered = filterTitles(titles, currentTab, currentFilter);
	sortTitles(filtered, currentSort);

	const int cols = computeColumns();
	const int spacing = computeSpacing();

	cardsLayout->setHorizontalSpacing(spacing);
	cardsLayout->setVerticalSpacing(CARD_SPACING);
	cardsLayout->setContentsMargins(spacing, 0, spacing, 0);

	int col = 0;
	int row = 0;

	for(const Title &t : filtered)
	{
		Title display = t;
		if(currentSort == SortMode::Rank)
			display.title = QStringLiteral("#%1 %2").arg(t.rank).arg(t.title);

		auto *card = new TitleCard(display, appStorage, cardWidth, cardsContainer);
		connect(card, &TitleCard::clicked, this, [this, t]() { emit titleClicked(t); });
		cardsLayout->addWidget(card, row, col);

		if(++col >= cols)
		{
			col = 0;
			++row;
		}
	}
}

void LibraryView::clear()
{
	while(QLayoutItem *item = cardsLayout->takeAt(0))
	{
		if(item->widget())
		{
			item->widget()->deleteLater();
		}

		delete item;
	}
}
