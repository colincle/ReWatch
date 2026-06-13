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

    void setSort(SortMode sort)       { currentSort   = sort;   populate(); }
    void setTab(LibraryTab tab)       { currentTab    = tab;    populate(); }
    void setFilter(ViewFilter filter) { currentFilter = filter; populate(); }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    static constexpr int CARD_WIDTH   = 160;
    static constexpr int CARD_SPACING = 16;
    static constexpr int MARGIN       = 20;

    AppStorage &appStorage;

    SortMode   currentSort   = SortMode::LastViewed;
    LibraryTab currentTab    = LibraryTab::Movies;
    ViewFilter currentFilter = ViewFilter::All;

    LibraryViewTopBar *libraryViewTopBar;
    QScrollArea       *scrollArea;
    QWidget           *cardsContainer;
    QGridLayout       *cardsLayout;
    QTimer            *resizeTimer;

    std::vector<Title> titles;

    void setupUi();
    void connectSignals();
    void populate();
    void clear();

    QWidget *makeTopBarWrapper();
    QWidget *makeScrollArea();

    void onSearchRequested(const QString &query);

    int computeColumns() const;
    int computeSpacing() const;
};