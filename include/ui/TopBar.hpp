#pragma once

#include "IconButton.hpp"
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
    TextButton *MoviesButton;
    TextButton *TvShowsButton;

    void connectButtons();

signals:
    void requestAddMode();
};