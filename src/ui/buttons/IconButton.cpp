#include "IconButton.hpp"
#include "SvgUtils.hpp"

#include <QEvent>
#include <QTimer>

IconButton::IconButton(const QString &iconPath, int size, QString color1, QString color2, QWidget *parent)
    : QPushButton(parent)
    , color1(color1)
    , color2(color2)
{
    const int iconSize = size / 1.5;

    normalIcon = loadColoredSvg(iconPath, color1, iconSize);
    hoverIcon  = loadColoredSvg(iconPath, color2, iconSize);

    setFixedSize(size, size);
    setIconSize(QSize(iconSize, iconSize));
    setCursor(Qt::PointingHandCursor);

    applyNormal();
}

QString IconButton::styleSheet(const QString &bgColor) const
{
    return
        "QPushButton {"
        "    background-color: " + bgColor + ";"
        "    border: none;"
        "    border-radius: 6px;"
        "}";
}

void IconButton::applyNormal()
{
    setIcon(normalIcon);
    setStyleSheet(styleSheet(color2));
}

void IconButton::applyHover()
{
    setIcon(hoverIcon);
    setStyleSheet(styleSheet(color1));
}

void IconButton::enterEvent(QEnterEvent *event)
{
    applyHover();
    QPushButton::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    applyNormal();
    QPushButton::leaveEvent(event);
}

void IconButton::showEvent(QShowEvent *event)
{
    QPushButton::showEvent(event);
    QTimer::singleShot(0, this, [this]() {
        if (underMouse())
            applyHover();
    });
}