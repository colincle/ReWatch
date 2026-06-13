#pragma once

#include "AppStorage.hpp"
#include "AddBar.hpp"
#include "LibraryView.hpp"
#include "SearchResults.hpp"
#include "TopBar.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    AppStorage     appStorage;
    TopBar        *topBar;
    AddBar        *addBar;
    SearchResults *searchResults;
    LibraryView   *libraryView;

    void setupLayout();
    void setupShortcuts();
    void setupMenuBar();
    void connectSignals();

    void enterAddMode();
    void enterNormalMode();

private slots:
    void onSetApiKeyTriggered();
};