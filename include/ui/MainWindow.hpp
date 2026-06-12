#pragma once

#include "TopBar.hpp"
#include "AddBar.hpp"
#include "AppStorage.hpp"
#include "SearchResults.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);

private:
    AppStorage appStorage;
    TopBar *topBar;
    AddBar *addBar;
    SearchResults *searchResults;

    void setupMenuBar();
    void connectMainWindow();

private slots:
    void onSetApiKeyTriggered();
};