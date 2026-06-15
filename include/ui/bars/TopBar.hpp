#pragma once

#include "IconButton.hpp"
#include "SortEnums.hpp"
#include "TextButton.hpp"

#include <QWidget>

class TopBar : public QWidget
{
	Q_OBJECT

public:
	explicit TopBar(QWidget *parent = nullptr);

private:
	IconButton *rankButton;
	IconButton *sortButton;
	IconButton *addButton;
	TextButton *moviesButton;
	TextButton *tvShowsButton;

	void setupLayout();
	void connectButtons();

	void onMoviesClicked();
	void onTvShowsClicked();
	void onSortClicked();

signals:
	void requestAddMode();
	void requestSort(SortMode sortMode);
	void requestTab(LibraryTab tab);
};