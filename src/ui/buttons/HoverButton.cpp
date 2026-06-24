// showEvent checks underMouse() deferred via singleShot(0) because the widget geometry
// isn't fully committed yet at the moment showEvent fires.
#include "HoverButton.hpp"

#include <QString>
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
	QTimer::singleShot(
	    0,
	    this,
	    [this]()
	    {
		    if(underMouse())
			    applyHover();
	    }
	);
}

void HoverButton::unhover()
{
	applyNormal();
}

QString HoverButton::buildStyleSheet(
    const QString &bgColor, const QString &textColor, int iconPadding, int rightPadding
)
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1;"
	           "    color: %2;"
	           "    border: none;"
	           "    border-radius: 6px;"
	           "    text-align: left;"
	           "    padding-left: %3px;"
	           "    padding-right: %4px;"
	           "}"
	)
	    .arg(bgColor, textColor)
	    .arg(iconPadding)
	    .arg(rightPadding);
}
