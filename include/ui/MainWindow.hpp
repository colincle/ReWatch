#pragma once

#include "AppStorage.hpp"
#include "AddBar.hpp"
#include "AppMenuBar.hpp"
#include "LibraryView.hpp"
#include "SearchResults.hpp"
#include "TitleDetailView.hpp"
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
	AppStorage              appStorage;
	AppMenuBar             *appMenuBar            = nullptr;
	ErrorCard              *errorCard              = nullptr;
	TopBar                 *topBar                 = nullptr;
	AddBar                 *addBar                 = nullptr;
	SearchResults          *searchResults          = nullptr;
	LibraryView            *libraryView            = nullptr;
	TitleDetailView        *titleDetailView        = nullptr;
	SeasonUpdateController *seasonUpdateController = nullptr;
	QWidget                *seasonOverlay = nullptr;

	void     buildUi();
	QWidget *makeSeasonOverlay();
	void     setupLayout();
	void     setupErrorCard();
	void     setupMenuBar();
	void     setupShortcuts();
	void     connectSignals();
	void     setupSeasonUpdateController();

	void enterAddMode();
	void enterNormalMode();
	void enterDetailMode(const Title &title);

	void resizeEvent(QResizeEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
};
