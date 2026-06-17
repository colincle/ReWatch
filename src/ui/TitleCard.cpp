#include "TitleCard.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"
#include "ElidedLabel.hpp"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

static constexpr int BTN_SIZE = 36;
static constexpr int BTN_MARGIN = 8;

static QPixmap roundedPixmap(const QPixmap &src, int radius)
{
	QPixmap result(src.size());
	result.fill(Qt::transparent);
	QPainter p(&result);
	p.setRenderHint(QPainter::Antialiasing);
	QPainterPath path;
	path.addRoundedRect(src.rect(), radius, radius);
	p.setClipPath(path);
	p.drawPixmap(0, 0, src);
	return result;
}

TitleCard::TitleCard(const Title &title, AppStorage &appStorage, int cardWidth, QWidget *parent)
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
	setStyleSheet(
	    "TitleCard {"
	    "    background-color: " COLOR_BG_SECONDARY ";"
	    "    border: 1px solid " COLOR_BORDER ";"
	    "}"
	);

	setupPosterLabel();
	setupTitleLabel();
	setupButtons();
	hideButtons();
	connectButtons();
}

void TitleCard::setupPosterLabel()
{
	posterLabel = new QLabel(this);
	posterLabel->setGeometry(0, 0, cardWidth, posterHeight);
	posterLabel->setStyleSheet("border: none; background: transparent;");
	posterLabel->setAlignment(Qt::AlignCenter);
	posterLabel->setPixmap(roundedPixmap(
	                           title.posterImage.scaled(
	                               QSize(cardWidth, posterHeight),
	                               Qt::KeepAspectRatioByExpanding,
	                               Qt::SmoothTransformation),
	                           10)
	                      );
}

void TitleCard::setupTitleLabel()
{
	titleLabel = new ElidedLabel(title.title, 2, this);
	titleLabel->setGeometry(0, posterHeight, cardWidth, titleLabelHeight);
	titleLabel->setStyleSheet(
	    "border: none; background: transparent;"
	    "color: " COLOR_TEXT_PRIMARY "; font-size: 12px;"
	);
	titleLabel->setAlignment(Qt::AlignCenter);
}

void TitleCard::setupButtons()
{
	viewedButton = new IconButton(VIEWED_ICON, BTN_SIZE, COLOR_SUCCESS, COLOR_SURFACE, this);
	notViewedButton = new IconButton(NOT_VIEWED_ICON, BTN_SIZE, COLOR_ERROR, COLOR_SURFACE, this);
	deleteButton = new IconButton(DELETE_ICON, BTN_SIZE, COLOR_ERROR, COLOR_SURFACE, this);
	uploadPosterButton = new IconButton(IMAGE_UPLOAD_ICON, BTN_SIZE, COLOR_ACCENT, COLOR_SURFACE, this);

	viewedButton->move(BTN_MARGIN, posterHeight - BTN_SIZE - BTN_MARGIN);
	notViewedButton->move(BTN_MARGIN, posterHeight - BTN_SIZE - BTN_MARGIN);
	deleteButton->move(cardWidth - BTN_SIZE - BTN_MARGIN, posterHeight - BTN_SIZE - BTN_MARGIN);
	uploadPosterButton->move(cardWidth - BTN_SIZE - BTN_MARGIN, BTN_MARGIN);

	uploadPosterButton->setVisible(title.posterNotFound);
}

void TitleCard::connectButtons()
{
	connect(viewedButton, &QPushButton::clicked, this, &TitleCard::onViewedClicked);
	connect(notViewedButton, &QPushButton::clicked, this, &TitleCard::onNotViewedClicked);
	connect(deleteButton, &QPushButton::clicked, this, &TitleCard::onDeleteClicked);
	connect(uploadPosterButton, &QPushButton::clicked, this, &TitleCard::onUploadPosterClicked);
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

	posterLabel->setPixmap(roundedPixmap(
	                           image.scaled(
	                               QSize(cardWidth, posterHeight),
	                               Qt::KeepAspectRatioByExpanding,
	                               Qt::SmoothTransformation),
	                           10)
	                      );

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
}

void TitleCard::hideButtons()
{
	viewedButton->hide();
	notViewedButton->hide();
	deleteButton->hide();
}