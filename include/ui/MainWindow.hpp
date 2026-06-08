#pragma once

#include "TopBar.hpp"
#include "AddBar.hpp"
#include "AppUtils.hpp"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);

private:
    AppUtils appUtils;
    TopBar *topBar;
    AddBar *addBar;

    void setupMenuBar();
    void connectMainWindow();
};