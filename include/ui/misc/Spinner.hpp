#pragma once

#include <QWidget>
#include <QTimer>
#include <QString>

class Spinner : public QWidget
{
    Q_OBJECT

public:
    explicit Spinner(
        QString color,
        int speed,
        QWidget *parent = nullptr
    );

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QTimer timer;

    QString color;
    double speed;

    int blockCell[3] = {0, 1, 2};
    int emptyCell = 3;

    int movingBlock = -1;
    int movingFrom = -1;
    int movingTo = -1;

    double progress = 0.0;
};