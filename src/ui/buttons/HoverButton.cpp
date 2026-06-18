#include "HoverButton.hpp"

#include <QTimer>

void HoverButton::enterEvent(QEnterEvent *event)
{
	applyHover();
	QPushButton::enterEvent(event);
}

void HoverButton::leaveEvent(QEvent *event)
{
	applyNormal();
	QPushButton::leaveEvent(event);
}

void HoverButton::showEvent(QShowEvent *event)
{
	QPushButton::showEvent(event);
	QTimer::singleShot(0, this, [this]() { if(underMouse()) applyHover(); });
}

void HoverButton::unhover()
{
	applyNormal();
}
