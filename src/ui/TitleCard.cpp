#include "TitleCard.hpp"
#include "AssetsPaths.hpp"
#include "Palette.hpp"
#include "ElidedLabel.hpp"

#include <QDir>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

static constexpr int BTN_SIZE = 36;
static constexpr int BTN_MARGIN = 8;
static constexpr int POSTER_RADIUS = 10;

TitleCard::TitleCard(
    const Title &title, AppStorage &appStorage, int cardWidth, QWidget *parent
)
    : QWidget(parent)
    , title(title)
    , appStorage(appStorage)
    , cardWidth(cardWidth)
    , posterHeight(cardWidth * 3 / 2)
    , titleLabelHeight(cardWidth / 5)
{
	setupUi();
}

void TitleCard::setupUi()
{
	setFixedSize(cardWidth, posterHeight + titleLabelHeight);
	setCursor(Qt::PointingHandCursor);

	posterPixmap = title.posterImage.scaled(
	    QSize(cardWidth, posterHeight),
	    Qt::KeepAspectRatioByExpanding,
	    Qt::SmoothTransformation
	);

	setupTitleLabel();
	setupButtons();
	hideButtons();
	connectButtons();
}

void TitleCard::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	p.save();
	QPainterPath clip;
	clip.addRoundedRect(
	    QRectF(0, 0, cardWidth, posterHeight),
	    POSTER_RADIUS,
	    POSTER_RADIUS
	);
	p.setClipPath(clip);
	const int x = (cardWidth - posterPixmap.width()) / 2;
	const int y = (posterHeight - posterPixmap.height()) / 2;
	p.drawPixmap(x, y, posterPixmap);
	p.restore();
}

void TitleCard::setupTitleLabel()
{
	titleLabel = new ElidedLabel(title.title, 2, this);
	titleLabel->setGeometry(0, posterHeight, cardWidth, titleLabelHeight);
	titleLabel->setStyleSheet(
	    QStringLiteral(
	        "border: none; background: transparent; color: %1; font-size: 12px;"
	    )
	        .arg(Palette::textPrimary)
	);
	titleLabel->setAlignment(Qt::AlignCenter);
}

void TitleCard::setupButtons()
{
	viewedButton = new IconTextButton(
	    AssetsPaths::boxNotCheckedIcon,
	    "To watch",
	    BTN_SIZE,
	    Palette::error,
	    Palette::surface,
	    false,
	    true,
	    this
	);
	notViewedButton = new IconTextButton(
	    AssetsPaths::boxCheckedIcon,
	    "Watched",
	    BTN_SIZE,
	    Palette::success,
	    Palette::surface,
	    false,
	    true,
	    this
	);
	deleteButton = new IconButton(
	    AssetsPaths::deleteIcon,
	    BTN_SIZE,
	    Palette::error,
	    Palette::surface,
	    this
	);
	uploadPosterButton = new IconButton(
	    AssetsPaths::imageUploadIcon,
	    BTN_SIZE,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	unrankButton = new IconButton(
	    AssetsPaths::unrankIcon,
	    BTN_SIZE,
	    Palette::accent,
	    Palette::surface,
	    this
	);

	viewedButton->move(BTN_MARGIN, posterHeight - BTN_SIZE - BTN_MARGIN);
	notViewedButton->move(BTN_MARGIN, posterHeight - BTN_SIZE - BTN_MARGIN);
	deleteButton->move(
	    cardWidth - BTN_SIZE - BTN_MARGIN,
	    posterHeight - BTN_SIZE - BTN_MARGIN
	);
	uploadPosterButton->move(cardWidth - BTN_SIZE - BTN_MARGIN, BTN_MARGIN);
	unrankButton->move(BTN_MARGIN, BTN_MARGIN);

	uploadPosterButton->setVisible(title.posterNotFound);
	unrankButton->setVisible(false);
}

void TitleCard::connectButtons()
{
	connect(viewedButton, &QPushButton::clicked, this, &TitleCard::onViewedClicked);
	connect(notViewedButton, &QPushButton::clicked, this, &TitleCard::onNotViewedClicked);
	connect(deleteButton, &QPushButton::clicked, this, &TitleCard::onDeleteClicked);
	connect(
	    uploadPosterButton,
	    &QPushButton::clicked,
	    this,
	    &TitleCard::onUploadPosterClicked
	);
	connect(
	    unrankButton,
	    &QPushButton::clicked,
	    this,
	    [this]()
	    {
		    appStorage.clearRank(title.imdbId);
		    title.rank = 0;
		    unrankButton->hide();
	    }
	);
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

void TitleCard::onUploadPosterClicked()
{
	const QString path = QFileDialog::getOpenFileName(
	    this,
	    "Choose Poster Image",
	    QDir::homePath(),
	    "Images (*.png *.jpg *.jpeg)"
	);

	if(path.isEmpty())
	{
		return;
	}

	QPixmap image(path);

	if(image.isNull())
	{
		return;
	}

	appStorage.setPoster(title.imdbId, image);

	title.posterImage = image;
	title.posterNotFound = false;

	posterPixmap = image.scaled(
	    QSize(cardWidth, posterHeight),
	    Qt::KeepAspectRatioByExpanding,
	    Qt::SmoothTransformation
	);

	update();
	uploadPosterButton->hide();
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
	if(event->button() == Qt::LeftButton)
	{
		emit clicked();
	}

	QWidget::mousePressEvent(event);
}

void TitleCard::showButtons()
{
	title.viewed ? notViewedButton->show() : viewedButton->show();
	deleteButton->show();
	deleteButton->raise();
	if(title.rank > 0)
		unrankButton->show();
}

void TitleCard::hideButtons()
{
	viewedButton->hide();
	notViewedButton->hide();
	deleteButton->hide();
	unrankButton->hide();
}
