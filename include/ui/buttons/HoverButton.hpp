#pragma once

#include <QPushButton>
#include <QString>

class HoverButton : public QPushButton
{
	Q_OBJECT

  public:
	using QPushButton::QPushButton;

	void unhover();

  protected:
	virtual void applyNormal() = 0;
	virtual void applyHover() = 0;

	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void showEvent(QShowEvent *event) override;

	static QString buildStyleSheet(
	    const QString &bgColor, const QString &textColor, int iconPadding,
	    int rightPadding = 12
	);
};
