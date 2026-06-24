// App-wide top bar with tab switcher, sort, rank, add, and notifications controls.
#pragma once

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "NotificationsCenter.hpp"
#include "SortEnums.hpp"
#include "TextButton.hpp"

#include <QWidget>

class TopBar : public QWidget
{
	Q_OBJECT

  public:
	explicit TopBar(AppStorage &appStorage, QWidget *parent = nullptr);

  public:
	void refreshStyle();

  private:
	AppStorage &appStorage;

	IconButton          *notificationsButton;
	QWidget             *notificationDot;
	NotificationsCenter *notificationsCenter;

	IconButton *rankButton;
	IconButton *sortButton;
	IconButton *addButton;
	TextButton *moviesButton;
	TextButton *tvShowsButton;

	void setupLayout();
	void connectButtons();
	void updateNotificationDot();

	void onMoviesClicked();
	void onTvShowsClicked();
	void onNotificationsClicked();
	void onSortClicked();
	void onRankClicked();

  signals:
	void requestAddMode();
	void requestSort(SortMode sortMode);
	void requestTab(LibraryTab tab);
	void titleNavigationRequested(const Title &title);
	void requestRanking();
};
