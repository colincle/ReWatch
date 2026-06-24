// Square icon-only button that swaps between two tinted SVG icons on hover.
#pragma once

#include "HoverButton.hpp"

#include <QIcon>

class IconButton : public HoverButton
{
  public:
	explicit IconButton(
	    const QString &iconPath, int size, QString color1, QString color2, QWidget *parent
	);

  public:
	void updateColors(const QString &c1, const QString &c2);

  protected:
	void applyNormal() override;
	void applyHover() override;

  private:
	QString iconPath;
	QIcon   normalIcon;
	QIcon   hoverIcon;
	QString color1;
	QString color2;

	QString styleSheet(const QString &bgColor) const;
};
