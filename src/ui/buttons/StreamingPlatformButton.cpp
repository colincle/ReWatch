#include "StreamingPlatformButton.hpp"
#include "Palette.hpp"

#include <QFont>
#include <QIcon>
#include <QPixmap>
#include <QTimer>

StreamingPlatformButton::StreamingPlatformButton(
    const StreamingPlatform &platform, int height, QString color1, QString color2,
    QWidget *parent
)
    : QPushButton(platform.name, parent), color1(color1), color2(color2)
{
	const int iconSize = height * 2 / 3;
	iconPadding = (height - iconSize) / 2;

	if(!platform.image.isEmpty())
	{
		QPixmap px(platform.image);
		if(!px.isNull())
		{
			QPixmap scaled = px.scaled(
			    iconSize,
			    iconSize,
			    Qt::KeepAspectRatioByExpanding,
			    Qt::SmoothTransformation
			);
			QPixmap cropped = scaled.copy(
			    (scaled.width() - iconSize) / 2,
			    (scaled.height() - iconSize) / 2,
			    iconSize,
			    iconSize
			);
			setIcon(QIcon(cropped));
			setIconSize(QSize(iconSize, iconSize));
		}
	}

	QFont f = font();
	f.setPixelSize(static_cast<int>(height * 0.4));
	setFont(f);

	setFixedHeight(height);
	setCursor(Qt::PointingHandCursor);

	applyNormal();
}

QString StreamingPlatformButton::buildStyleSheet(
    const QString &bgColor, const QString &textColor
) const
{
	return QStringLiteral(
	           "QPushButton {"
	           "    background-color: %1;"
	           "    color: %2;"
	           "    border: none;"
	           "    border-radius: 6px;"
	           "    text-align: left;"
	           "    padding-left: %3px;"
	           "    padding-right: 12px;"
	           "}"
	)
	    .arg(bgColor, textColor)
	    .arg(iconPadding);
}

void StreamingPlatformButton::applyNormal()
{
	setStyleSheet(buildStyleSheet(color2, color1));
}

void StreamingPlatformButton::applyHover()
{
	setStyleSheet(buildStyleSheet(color1, color2));
}

void StreamingPlatformButton::enterEvent(QEnterEvent *event)
{
	applyHover();
	QPushButton::enterEvent(event);
}

void StreamingPlatformButton::leaveEvent(QEvent *event)
{
	applyNormal();
	QPushButton::leaveEvent(event);
}

void StreamingPlatformButton::showEvent(QShowEvent *event)
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
