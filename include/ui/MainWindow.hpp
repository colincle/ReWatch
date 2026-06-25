// Root window. Owns AppStorage and all top-level views; routes mode transitions
// (normal / add / detail / ranking) and wires the background library update.
#pragma once

#include "AppStorage.hpp"
#include "AddBar.hpp"
#include "AppMenuBar.hpp"
#include "LibraryView.hpp"
#include "RankingView.hpp"
#include "SearchResults.hpp"
#include "TitleDetailView.hpp"
#include "TopBar.hpp"
#include "ErrorCard.hpp"
#include "LibraryUpdateController.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	MainWindow(QWidget *parent = nullptr);

  private:
	AppStorage               appStorage;
	AppMenuBar              *appMenuBar = nullptr;
	ErrorCard               *errorCard = nullptr;
	TopBar                  *topBar = nullptr;
	AddBar                  *addBar = nullptr;
	SearchResults           *searchResults = nullptr;
	LibraryView             *libraryView = nullptr;
	TitleDetailView         *titleDetailView = nullptr;
	LibraryUpdateController *libraryUpdateController = nullptr;
	RankingView             *rankingView = nullptr;

	void buildUi();
	void onStyleChanged();
	void setupLayout();
	void setupErrorCard();
	void setupMenuBar();
	void setupShortcuts();
	void connectSignals();
	void setupLibraryUpdateController();

	void enterAddMode();
	void enterNormalMode();
	void enterDetailMode(const Title &title);
	void startRanking();

	void resizeEvent(QResizeEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
};
