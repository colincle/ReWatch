#pragma once

#include "AppStorage.hpp"
#include "AddBar.hpp"
#include "LibraryView.hpp"
#include "SearchResults.hpp"
#include "TopBar.hpp"
#include "ErrorCard.hpp"
#include "SeasonUpdate.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

private:
	AppStorage appStorage;
	ErrorCard *errorCard;
	TopBar *topBar;
	AddBar *addBar;
	SearchResults *searchResults;
	LibraryView *libraryView;

	QWidget *makeSeasonOverlay();
	void setupLayout();
	void setupErrorCard();
	void setupShortcuts();
	void setupMenuBar();
	void connectSignals();
	void runSeasonUpdate();

	void enterAddMode();
	void enterNormalMode();

private slots:
	void onSetApiKeyTriggered();
};
