#include "AddBar.hpp"
#include "IconButton.hpp"
#include "ColorPalette.hpp"
#include "AssetsPaths.hpp"

#include <QHBoxLayout>
#include <QKeyEvent>

AddBar::AddBar(QWidget *parent)
    : QWidget(parent)
{
    int buttonsHeight = 40;

    setStyleSheet(
        "background-color: " COLOR_BG_SECONDARY ";"
        "border-bottom: 1px solid " COLOR_BORDER ";"
    );
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(10);

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText(" Search...");
    searchBar->setClearButtonEnabled(true);
    searchBar->installEventFilter(this);

    searchBar->setFixedHeight(buttonsHeight);
    searchBar->setFrame(false);

    searchBar->setStyleSheet(
        "QLineEdit {"
        "    background-color: " COLOR_SURFACE ";"
        "    color: " COLOR_TEXT_SECONDARY ";"

        "    border: 1px solid " COLOR_BORDER ";"
        "    border-radius: 10px;"

        "    padding-left: 12px;"
        "    padding-right: 28px;"

        "    selection-background-color: " COLOR_ACCENT_LIGHT ";"
        "    selection-color: white;"
        "}"
    );
    returnButton = new IconButton(CROSS_ICON, buttonsHeight, this);

    layout->addWidget(searchBar, 1);
    layout->addWidget(returnButton);

    connectBar();
}

bool AddBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == searchBar && event->type() == QEvent::KeyPress)
    {
        auto *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            searchBar->clear();
            emit requestNormalMode();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void AddBar::connectBar()
{
    connect(returnButton, &QPushButton::clicked, this, [this]() {
        emit requestNormalMode();
    });

    connect(searchBar, &QLineEdit::returnPressed, this, [this]() {
        QString query = searchBar->text().trimmed();

        if (query.isEmpty())
            return;

        emit searchRequested(query);
    });
}