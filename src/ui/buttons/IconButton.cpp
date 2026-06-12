#include "IconButton.hpp"
#include "ColorPalette.hpp"
#include "SvgUtils.hpp"

#include <QEvent>
#include <string>

IconButton::IconButton(const QString &iconPath, int size, QString color1, QString color2, QWidget *parent)
    : QPushButton(parent)
{
    int buttonSize = size;
    int iconSize = size / 1.5;

    normalIcon = loadColoredSvg(iconPath, color1, iconSize);
    hoverIcon  = loadColoredSvg(iconPath, color2, iconSize);

    setIcon(normalIcon);
    setFixedSize(buttonSize, buttonSize);
    setIconSize(QSize(iconSize, iconSize));
    setCursor(Qt::PointingHandCursor);

    setStyleSheet(
        "QPushButton {"
        "    background-color: " + color2 + ";"
        "    border: none;"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: " + color1 + ";"
        "    border-radius: 6px;"
        "}"
    );
}

void IconButton::enterEvent(QEnterEvent *event)
{
    setIcon(hoverIcon);
    QPushButton::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    setIcon(normalIcon);
    QPushButton::leaveEvent(event);
}