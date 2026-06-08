#pragma once

#include "IconButton.hpp"

#include <QWidget>
#include <QLineEdit>

class AddBar : public QWidget
{
    Q_OBJECT
public:
    explicit AddBar(QWidget *parent = nullptr);

private:
    IconButton *returnButton;
    QLineEdit *searchBar;

    bool eventFilter(QObject *obj, QEvent *event);
    void connectBar();

signals:
    void requestNormalMode();
    void searchRequested(const QString &query);
};