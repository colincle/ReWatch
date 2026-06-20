#include "IconButton.hpp"
#include "SvgUtils.hpp"

#include <utility>

IconButton::IconButton(
    const QString &iconPath, int size, QString color1, QString color2, QWidget *parent
)
    : HoverButton(parent)
    , iconPath(iconPath)
    , color1(std::move(color1))
    , color2(std::move(color2))
{
	const int iconSize = size / 1.5;

	normalIcon = loadColoredSvg(iconPath, this->color1, iconSize);
	hoverIcon = loadColoredSvg(iconPath, this->color2, iconSize);

	setFixedSize(size, size);
	setIconSize(QSize(iconSize, iconSize));

	applyNormal();
}

QString IconButton::styleSheet(const QString &bgColor) const
{
	return "QPushButton {"
	       "    background-color: " +
	       bgColor +
	       ";"
	       "    border: none;"
	       "    border-radius: 6px;"
	       "}";
}

void IconButton::updateColors(const QString &c1, const QString &c2)
{
	color1 = c1;
	color2 = c2;
	const int iSize = iconSize().width();
	normalIcon = loadColoredSvg(iconPath, color1, iSize);
	hoverIcon = loadColoredSvg(iconPath, color2, iSize);
	applyNormal();
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
