#pragma once

#include "AppStorage.hpp"
#include "AddBar.hpp"
#include "AppMenuBar.hpp"
#include "LibraryView.hpp"
#include "SearchResults.hpp"
#include "TopBar.hpp"
#include "ErrorCard.hpp"
#include "SeasonUpdateController.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

private:
	AppStorage appStorage;
	AppMenuBar *appMenuBar;
	ErrorCard *errorCard;
	TopBar *topBar;
	AddBar *addBar;
	SearchResults *searchResults;
	LibraryView *libraryView;
	SeasonUpdateController *seasonUpdateController;
	QWidget *seasonOverlay = nullptr;

	QWidget *makeSeasonOverlay();
	void setupLayout();
	void setupErrorCard();
	void setupMenuBar();
	void setupShortcuts();
	void connectSignals();
	void setupSeasonUpdateController();

	void enterAddMode();
	void enterNormalMode();

	void closeEvent(QCloseEvent *event) override;
};
