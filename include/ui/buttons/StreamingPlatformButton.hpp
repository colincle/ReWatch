#pragma once

#include "AppStorage.hpp"

#include <QPushButton>

class StreamingPlatformButton : public QPushButton
{
  public:
	explicit StreamingPlatformButton(
	    const StreamingPlatform &platform, int height, QString color1, QString color2,
	    QWidget *parent = nullptr
	);

  protected:
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void showEvent(QShowEvent *event) override;

  private:
	QString color1;
	QString color2;
	int     iconPadding;

	void    applyNormal();
	void    applyHover();
	QString buildStyleSheet(const QString &bgColor, const QString &textColor) const;
};
