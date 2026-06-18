#pragma once

#include "HoverButton.hpp"

#include <QIcon>

class IconButton : public HoverButton
{
  public:
	explicit IconButton(
	    const QString &iconPath, int size, QString color1, QString color2, QWidget *parent
	);

  protected:
	void applyNormal() override;
	void applyHover() override;

  private:
	QIcon   normalIcon;
	QIcon   hoverIcon;
	QString color1;
	QString color2;

	QString styleSheet(const QString &bgColor) const;
};
