#include "TitleCard.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>

static constexpr int CARD_W     = 160;
static constexpr int CARD_H     = 240;
static constexpr int BTN_SIZE   = 36;
static constexpr int BTN_MARGIN = 8;

TitleCard::TitleCard(const Title &title, AppStorage &appStorage, QWidget *parent)
    : QWidget(parent)
    , title(title)
    , appStorage(appStorage)
{
    setupUi();
}

void TitleCard::setupUi()
{
    setFixedSize(CARD_W, CARD_H);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(
        "TitleCard {"
        "    background-color: " COLOR_BG_SECONDARY ";"
        "    border: 1px solid " COLOR_BORDER ";"
        "    border-radius: 10px;"
        "}"
    );

    posterLabel = new QLabel(this);
    posterLabel->setGeometry(0, 0, CARD_W, CARD_H);
    posterLabel->setStyleSheet("border: none; background: transparent;");
    posterLabel->setAlignment(Qt::AlignCenter);
    posterLabel->setPixmap(
        title.posterImage.scaled(
            QSize(CARD_W, CARD_H),
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation)
    );

    viewedButton = new IconButton(VIEWED_ICON,     BTN_SIZE, COLOR_SUCCESS, COLOR_SURFACE, this);
    notViewedButton = new IconButton(NOT_VIEWED_ICON, BTN_SIZE, COLOR_ERROR,   COLOR_SURFACE, this);
    deleteButton = new IconButton(DELETE_ICON,     BTN_SIZE, COLOR_ERROR,   COLOR_SURFACE, this);

    viewedButton->move(BTN_MARGIN,              CARD_H - BTN_SIZE - BTN_MARGIN);
    notViewedButton->move(BTN_MARGIN,           CARD_H - BTN_SIZE - BTN_MARGIN);
    deleteButton->move(CARD_W - BTN_SIZE - BTN_MARGIN, CARD_H - BTN_SIZE - BTN_MARGIN);

    hideButtons();
    connectButtons();
}

void TitleCard::connectButtons()
{
    connect(viewedButton,    &QPushButton::clicked, this, &TitleCard::onViewedClicked);
    connect(notViewedButton, &QPushButton::clicked, this, &TitleCard::onNotViewedClicked);
    connect(deleteButton,    &QPushButton::clicked, this, &TitleCard::onDeleteClicked);
}

void TitleCard::onViewedClicked()
{
    appStorage.toggleViewed(title.imdbId);
    title.viewed = true;
    viewedButton->hide();
    notViewedButton->show();
    notViewedButton->raise();
}

void TitleCard::onNotViewedClicked()
{
    appStorage.toggleViewed(title.imdbId);
    title.viewed = false;
    notViewedButton->hide();
    viewedButton->show();
    viewedButton->raise();
}

void TitleCard::onDeleteClicked()
{
    appStorage.deleteTitle(title.imdbId);
}

void TitleCard::enterEvent(QEnterEvent *event)
{
    showButtons();
    QWidget::enterEvent(event);
}

void TitleCard::leaveEvent(QEvent *event)
{
    hideButtons();
    QWidget::leaveEvent(event);
}

void TitleCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();

    QWidget::mousePressEvent(event);
}

void TitleCard::showButtons()
{
    title.viewed ? notViewedButton->show() : viewedButton->show();
    deleteButton->show();
    deleteButton->raise();
}

void TitleCard::hideButtons()
{
    viewedButton->hide();
    notViewedButton->hide();
    deleteButton->hide();
}