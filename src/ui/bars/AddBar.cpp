#include "AddBar.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"
#include "IconButton.hpp"

#include <QHBoxLayout>
#include <QKeyEvent>

static constexpr int BUTTON_HEIGHT = 40;

AddBar::AddBar(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(
        "background-color: " COLOR_BG_SECONDARY ";"
        "border-bottom: 1px solid " COLOR_BORDER ";"
    );
    setAttribute(Qt::WA_StyledBackground, true);

    setupLayout();
    connectSignals();
}

void AddBar::setupLayout()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(10);

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText(" Search...");
    searchBar->setClearButtonEnabled(true);
    searchBar->setFixedHeight(BUTTON_HEIGHT);
    searchBar->setFrame(false);
    searchBar->installEventFilter(this);
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

    returnButton = new IconButton(CROSS_ICON, BUTTON_HEIGHT, COLOR_ACCENT, COLOR_SURFACE, this);

    layout->addWidget(searchBar, 1);
    layout->addWidget(returnButton);
}

void AddBar::connectSignals()
{
    connect(returnButton, &QPushButton::clicked,     this, &AddBar::onReturnClicked);
    connect(searchBar,    &QLineEdit::returnPressed, this, &AddBar::onSearchCommitted);
}

void AddBar::onReturnClicked()
{
    emit requestNormalMode();
}

void AddBar::onSearchCommitted()
{
    QString query = searchBar->text().trimmed();
    if (!query.isEmpty())
        emit searchRequested(query);
}

bool AddBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == searchBar && event->type() == QEvent::KeyPress)
    {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            searchBar->clear();
            emit requestNormalMode();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}