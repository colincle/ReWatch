#pragma once

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "Title.hpp"

#include <QLabel>
#include <QWidget>

class TitleCard : public QWidget
{
    Q_OBJECT

public:
    explicit TitleCard(const Title &title, AppStorage &appStorage, QWidget *parent = nullptr);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void clicked();

private:
    Title       title;
    AppStorage &appStorage;

    QLabel     *posterLabel;
    IconButton *notViewedButton;
    IconButton *viewedButton;
    IconButton *deleteButton;

    void setupUi();
    void connectButtons();
    void showButtons();
    void hideButtons();

    void onViewedClicked();
    void onNotViewedClicked();
    void onDeleteClicked();
};