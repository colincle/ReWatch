#include "ErrorCard.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"

#include <QHBoxLayout>

static constexpr int CARD_MIN_HEIGHT = 48;
static constexpr int CARD_WIDTH = 360;
static constexpr int CARD_RADIUS = 10;
static constexpr int CARD_PADDING_LEFT = 12;
static constexpr int CARD_PADDING_RIGHT = 8;
static constexpr int CARD_PADDING_VERTICAL = 10;
static constexpr int CARD_ICON_SIZE = 24;
static constexpr int CARD_SPACING = 8;
static constexpr int LABEL_WIDTH =
    CARD_WIDTH - CARD_PADDING_LEFT - CARD_PADDING_RIGHT - CARD_SPACING - CARD_ICON_SIZE;

ErrorCard::ErrorCard(QWidget *parent, const QString &message) : QWidget(parent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setFixedWidth(CARD_WIDTH);
	setStyleSheet(QStringLiteral("background-color: %1; border-radius: %2px;")
	                  .arg(Palette::error)
	                  .arg(CARD_RADIUS));

	auto *layout = new QHBoxLayout(this);
	layout->setContentsMargins(
	    CARD_PADDING_LEFT,
	    CARD_PADDING_VERTICAL,
	    CARD_PADDING_RIGHT,
	    CARD_PADDING_VERTICAL
	);
	layout->setSpacing(CARD_SPACING);

	label = new QLabel(this);
	label->setStyleSheet(
	    "color: white; border: none; background: transparent; font-size: 13px;"
	);
	label->setWordWrap(true);
	label->setTextFormat(Qt::RichText);
	label->setOpenExternalLinks(true);
	label->setText(message);

	auto *closeButton = new IconButton(
	    AssetsPaths::crossIcon,
	    CARD_ICON_SIZE,
	    "white",
	    Palette::error,
	    this
	);

	layout->addWidget(label, 1);
	layout->addWidget(closeButton, 0, Qt::AlignTop);

	connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);

	updateHeight();
	hide();
}

void ErrorCard::show()
{
	QWidget::show();
	raise();
}

void ErrorCard::setMessage(const QString &message)
{
	label->setText(message);
	updateHeight();
}

void ErrorCard::updateHeight()
{
	const int textHeight = label->heightForWidth(LABEL_WIDTH);
	setFixedHeight(qMax(CARD_MIN_HEIGHT, textHeight + 2 * CARD_PADDING_VERTICAL));
}
