#include "ErrorCard.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"

#include <QHBoxLayout>

static constexpr int CARD_HEIGHT = 48;
static constexpr int CARD_WIDTH = 320;
static constexpr int CARD_RADIUS = 10;
static constexpr int CARD_PADDING_LEFT = 12;
static constexpr int CARD_PADDING_RIGHT = 8;
static constexpr int CARD_ICON_SIZE = 24;
static constexpr int CARD_SPACING = 8;

ErrorCard::ErrorCard(QWidget *parent, const QString &message)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setFixedSize(CARD_WIDTH, CARD_HEIGHT);
	setStyleSheet(
	    "background-color: " COLOR_ERROR ";"
	    "border-radius: " + QString::number(CARD_RADIUS) + "px;"
	);

	auto *layout = new QHBoxLayout(this);
	layout->setContentsMargins(CARD_PADDING_LEFT, 0, CARD_PADDING_RIGHT, 0);
	layout->setSpacing(CARD_SPACING);

	auto *label = new QLabel(message, this);
	label->setStyleSheet("color: white; border: none; background: transparent; font-size: 13px;");
	label->setWordWrap(true);

	auto *closeButton = new IconButton(CROSS_ICON, CARD_ICON_SIZE, "white", COLOR_ERROR, this);

	layout->addWidget(label);
	layout->addStretch();
	layout->addWidget(closeButton);

	connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);

	hide();
}

void ErrorCard::show()
{
	QWidget::show();
	raise();
}
