#pragma once

#include <QPushButton>

class HoverButton : public QPushButton
{
	Q_OBJECT

  public:
	using QPushButton::QPushButton;

	void unhover();

  protected:
	virtual void applyNormal() = 0;
	virtual void applyHover()  = 0;

	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void showEvent(QShowEvent *event) override;
};
