#include "StreamingPlatformButton.hpp"
#include "Palette.hpp"

#include <QFont>
#include <utility>
#include <QIcon>
#include <QPixmap>

StreamingPlatformButton::StreamingPlatformButton(
    const StreamingPlatform &platform, int height, QString color1, QString color2,
    QWidget *parent
)
    : HoverButton(platform.name, parent)
    , color1(std::move(color1))
    , color2(std::move(color2))
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

	applyNormal();
}

void StreamingPlatformButton::applyNormal()
{
	setStyleSheet(buildStyleSheet(color2, color1, iconPadding));
}

void StreamingPlatformButton::applyHover()
{
	setStyleSheet(buildStyleSheet(color1, color2, iconPadding));
}
