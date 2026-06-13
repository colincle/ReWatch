#pragma once

#include <QIcon>
#include <QPushButton>

class IconButton : public QPushButton
{
public:
    explicit IconButton(const QString &iconPath, int size, QString color1, QString color2, QWidget *parent);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QIcon   normalIcon;
    QIcon   hoverIcon;
    QString color1;
    QString color2;

    QString styleSheet(const QString &bgColor) const;
    void    applyNormal();
    void    applyHover();
};