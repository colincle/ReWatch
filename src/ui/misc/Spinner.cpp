#include "Spinner.hpp"

#include <QPainter>
#include <QColor>

Spinner::Spinner(
    QString color,
    int speed,
    QWidget *parent
)
    : QWidget(parent)
    , color(color)
{
    speed = qBound(1, speed, 10);

    auto startNextMove = [this]()
    {
        int nextEmpty = (emptyCell + 3) % 4;

        for (int i = 0; i < 3; ++i)
        {
            if (blockCell[i] == nextEmpty)
            {
                movingBlock = i;
                movingFrom = nextEmpty;
                movingTo = emptyCell;
                return;
            }
        }
    };

    startNextMove();

    double progressPerFrame = speed * 0.005;

    connect(&timer, &QTimer::timeout, this,
            [this, startNextMove, progressPerFrame]()
    {
        progress += progressPerFrame;

        if (progress >= 1.0)
        {
            blockCell[movingBlock] = movingTo;
            emptyCell = movingFrom;

            progress = 0.0;

            startNextMove();
        }

        update();
    });

    timer.start(16);

    setMinimumSize(24, 24);
}

void Spinner::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(color));

    const qreal size = qMin(width(), height());

    const qreal gap = size * 0.08;
    const qreal square = (size - gap) / 2.0;

    QPointF cells[4] =
    {
        {0, 0},
        {square + gap, 0},
        {square + gap, square + gap},
        {0, square + gap}
    };

    for (int i = 0; i < 3; ++i)
    {
        QPointF pos;

        if (i == movingBlock)
        {
            QPointF start = cells[movingFrom];
            QPointF end   = cells[movingTo];

            pos = start * (1.0 - progress)
                + end * progress;
        }
        else
        {
            pos = cells[blockCell[i]];
        }

        p.drawRoundedRect(
            QRectF(
                pos.x(),
                pos.y(),
                square,
                square
            ),
            square * 0.1,
            square * 0.1
        );
    }
}