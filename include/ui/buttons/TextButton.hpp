#pragma once

#include <QPushButton>

class TextButton : public QPushButton
{
  public:
	explicit TextButton(
	    const QString &text, int size, QString color1, QString color2,
	    QWidget *parent = nullptr
	);

	void toggleActive();
	bool isActive() const;

  protected:
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;

  private:
	QString color1;
	QString color2;
	bool    active = false;

	QString normalStyle() const;
	QString activeStyle() const;
};