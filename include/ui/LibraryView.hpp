#pragma once

#include "AppStorage.hpp"
#include "LibraryViewTopBar.hpp"
#include "SortEnums.hpp"
#include "TitleCard.hpp"

#include <QGridLayout>
#include <QScrollArea>
#include <QTimer>
#include <QWidget>
#include <vector>

class LibraryView : public QWidget
{
	Q_OBJECT

  public:
	explicit LibraryView(AppStorage &appStorage, QWidget *parent = nullptr);

	void refreshStyle();

	void applySort(SortMode sort)
	{
		currentSort = sort;
		populate();
	}
	void applyTab(LibraryTab tab)
	{
		currentTab = tab;
		populate();
	}
	void applyFilter(ViewFilter filter)
	{
		currentFilter = filter;
		populate();
	}

  protected:
	void resizeEvent(QResizeEvent *event) override;

  signals:
	void titleClicked(const Title &title);

  private:
	int                  cardWidth = 160;
	static constexpr int MIN_CARD_WIDTH = 170;
	static constexpr int MAX_CARD_WIDTH = 300;
	static constexpr int CARD_SPACING = 16;
	static constexpr int MARGIN = 20;

	AppStorage &appStorage;

	SortMode   currentSort = SortMode::WatchDate;
	LibraryTab currentTab = LibraryTab::Movies;
	ViewFilter currentFilter = ViewFilter::All;

	LibraryViewTopBar *libraryViewTopBar;
	QScrollArea       *scrollArea;
	QWidget           *cardsContainer;
	QGridLayout       *cardsLayout;
	QTimer            *resizeTimer;

	std::vector<Title> titles;
	QString            currentQuery;

	void setupUi();
	void connectSignals();
	void populate();
	void clear();

	QWidget *makeTopBarWrapper();
	QWidget *makeScrollArea();

	void onSearchRequested(const QString &query);
	void onZoomRequested(int zoomValue);

	int computeColumns() const;
	int computeSpacing() const;
};