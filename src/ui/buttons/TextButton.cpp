#include "TextButton.hpp"
#include "ColorPalette.hpp"

#include <QFont>

static QString makeStyle(const QString &bg, const QString &fg)
{
    return
        "QPushButton {"
        "   background-color: " + bg + ";"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 0px 14px;"
        "   color: " + fg + ";"
        "}";
}

TextButton::TextButton(const QString &text, int size, QWidget *parent)
    : QPushButton(text, parent)
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setFixedHeight(size);
    setContentsMargins(0, 0, 0, 0);

    QFont f = font();
    f.setPixelSize(static_cast<int>(size * 0.4));
    setFont(f);

    setStyleSheet(normalStyle());
}

QString TextButton::normalStyle() const
{
    return makeStyle(COLOR_SURFACE, COLOR_ACCENT);
}

QString TextButton::activeStyle() const
{
    return makeStyle(COLOR_ACCENT, COLOR_SURFACE);
}

void TextButton::toggleActive()
{
    active = !active;
    setStyleSheet(active ? activeStyle() : normalStyle());
}

bool TextButton::isActive() const
{
    return active;
}

void TextButton::enterEvent(QEnterEvent *event)
{
    if (!active)
        setStyleSheet(activeStyle());

    QPushButton::enterEvent(event);
}

void TextButton::leaveEvent(QEvent *event)
{
    if (!active)
        setStyleSheet(normalStyle());

    QPushButton::leaveEvent(event);
}