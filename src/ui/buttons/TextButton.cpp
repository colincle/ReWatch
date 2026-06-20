#include "TextButton.hpp"
#include "Palette.hpp"

#include <QFont>
#include <utility>

static QString makeStyle(const QString &bg, const QString &fg)
{
	return "QPushButton {"
	       "   background-color: " +
	       bg +
	       ";"
	       "   border: none;"
	       "   border-radius: 6px;"
	       "   padding: 0px 14px;"
	       "   color: " +
	       fg +
	       ";"
	       "}";
}

TextButton::TextButton(
    const QString &text, int size, QString color1, QString color2, QWidget *parent
)
    : QPushButton(text, parent), color1(std::move(color1)), color2(std::move(color2))
{

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
	return makeStyle(color2, color1);
}

QString TextButton::activeStyle() const
{
	return makeStyle(color1, color2);
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

void TextButton::updateColors(const QString &c1, const QString &c2)
{
	color1 = c1;
	color2 = c2;
	setStyleSheet(active ? activeStyle() : normalStyle());
}

void TextButton::enterEvent(QEnterEvent *event)
{
	if(!active)
	{
		setStyleSheet(activeStyle());
	}

	QPushButton::enterEvent(event);
}

void TextButton::leaveEvent(QEvent *event)
{
	if(!active)
	{
		setStyleSheet(normalStyle());
	}

	QPushButton::leaveEvent(event);
}