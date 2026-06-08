#pragma once

#include <QPushButton>
#include <QEvent>

class TextButton : public QPushButton
{
public:
    explicit TextButton(const QString &text,
                        int size,
                        QWidget *parent = nullptr);
    
    void toggleActive();
    bool isActive();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:

    void applyStyle();

    bool active = false;

    QString normalStyle;
    QString activeStyle;
    QString hoverStyle;
};