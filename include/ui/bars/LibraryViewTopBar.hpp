// Secondary bar inside the library view with filter tabs, inline search, and zoom controls.
#pragma once

#include "IconButton.hpp"
#include "SearchBar.hpp"
#include "SortEnums.hpp"
#include "TextButton.hpp"

#include <QWidget>

class LibraryViewTopBar : public QWidget
{
	Q_OBJECT

  public:
	explicit LibraryViewTopBar(QWidget *parent = nullptr);
	void refreshStyle();

  signals:
	void searchRequested(const QString &query);
	void filterChanged(ViewFilter filter);
	void zoomRequested(int zoomValue);

  private:
	TextButton *showAllButton;
	TextButton *showToWatchButton;

	IconButton *searchButton;
	IconButton *closeButton;
	IconButton *zoomInButton;
	IconButton *zoomOutButton;
	SearchBar  *searchInput;

	void setupLayout();
	void connectSignals();

	void onShowAllClicked();
	void onShowToWatchClicked();
	void onSearchCommitted();
	void onSearchTextChanged(const QString &text);

	void openSearch();
	void closeSearch();
};