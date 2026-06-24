// Icon button that animates to reveal a text label on hover.
#pragma once

#include "HoverButton.hpp"

#include <QIcon>
#include <QVariantAnimation>

class IconTextButton : public HoverButton
{
  public:
	explicit IconTextButton(
	    const QString &iconPath, const QString &text, int size, QString color1,
	    QString color2, bool alwaysShowText, bool textOnLeft = false,
	    QWidget *parent = nullptr
	);

  public:
	void updateColors(const QString &c1, const QString &c2);

  protected:
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void showEvent(QShowEvent *event) override;

  private:
	QString            iconPath;
	QIcon              normalIcon;
	QIcon              hoverIcon;
	QString            color1;
	QString            color2;
	QString            displayText;
	bool               alwaysShowText;
	int                collapsedWidth;
	int                expandedWidth;
	int                iconPadding;
	QVariantAnimation *animation = nullptr;

	void applyNormal() override;
	void applyHover() override;
	void animateTo(int targetWidth);
};
