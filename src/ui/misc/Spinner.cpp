#include "Spinner.hpp"

#include <QPainter>

Spinner::Spinner(QString color, int speed, QWidget *parent)
    : QWidget(parent)
    , color(color)
{
    speed = qBound(1, speed, 10);

    const double progressPerFrame = speed * 0.005;

    prepareNextMove();

    connect(&timer, &QTimer::timeout, this, [this, progressPerFrame]() {
        onTick(progressPerFrame);
    });

    timer.start(16);
    setMinimumSize(24, 24);
}

void Spinner::prepareNextMove()
{
    int nextEmpty = (emptyCell + 3) % 4;

    for (int i = 0; i < 3; ++i)
    {
        if (blockCell[i] == nextEmpty)
        {
            movingBlock = i;
            movingFrom  = nextEmpty;
            movingTo    = emptyCell;
            return;
        }
    }
}

void Spinner::onTick(double progressPerFrame)
{
    progress += progressPerFrame;

    if (progress >= 1.0)
    {
        blockCell[movingBlock] = movingTo;
        emptyCell              = movingFrom;
        progress               = 0.0;
        prepareNextMove();
    }

    update();
}

void Spinner::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(color));

    const qreal size   = qMin(width(), height());
    const qreal gap    = size * 0.08;
    const qreal square = (size - gap) / 2.0;

    const QPointF cells[4] = {
        {0,             0          },
        {square + gap,  0          },
        {square + gap,  square + gap},
        {0,             square + gap},
    };

    for (int i = 0; i < 3; ++i)
    {
        QPointF pos = (i == movingBlock)
            ? cells[movingFrom] * (1.0 - progress) + cells[movingTo] * progress
            : cells[blockCell[i]];

        p.drawRoundedRect(
            QRectF(pos.x(), pos.y(), square, square),
            square * 0.1,
            square * 0.1
        );
    }
}