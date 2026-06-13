#pragma once

#include "IconButton.hpp"

#include <QLineEdit>
#include <QWidget>

class AddBar : public QWidget
{
    Q_OBJECT

public:
    explicit AddBar(QWidget *parent = nullptr);

signals:
    void requestNormalMode();
    void searchRequested(const QString &query);

private:
    IconButton *returnButton;
    QLineEdit  *searchBar;

    void setupLayout();
    void connectSignals();

    void onReturnClicked();
    void onSearchCommitted();

    bool eventFilter(QObject *obj, QEvent *event) override;
};