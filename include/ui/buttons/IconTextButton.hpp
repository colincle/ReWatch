#pragma once

#include <QIcon>
#include <QPushButton>
#include <QVariantAnimation>

class IconTextButton : public QPushButton
{
  public:
	explicit IconTextButton(
	    const QString &iconPath, const QString &text, int size, QString color1,
	    QString color2, bool alwaysShowText, bool textOnLeft = false,
	    QWidget *parent = nullptr
	);

  protected:
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void showEvent(QShowEvent *event) override;

  private:
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

	QString buildStyleSheet(const QString &bgColor, const QString &textColor) const;
	void    applyNormal();
	void    applyHover();
	void    animateTo(int targetWidth);
};
