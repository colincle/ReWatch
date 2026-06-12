#pragma once

#include <QPushButton>
#include <QIcon>

class IconButton : public QPushButton
{
public:
    explicit    IconButton(const QString &iconPath, int size, QString color1, QString color2, QWidget *parent);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QIcon normalIcon;
    QIcon hoverIcon;
};