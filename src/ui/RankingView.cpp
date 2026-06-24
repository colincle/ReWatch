// Binary-search ranking: low/high converge on the insertion point with each pick.
// rankedTitlesOfType reads from storage each call so it reflects any concurrent updates.
#include "RankingView.hpp"
#include "AssetsPaths.hpp"
#include "ElidedLabel.hpp"
#include "IconButton.hpp"
#include "Palette.hpp"

#include <algorithm>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QVBoxLayout>

static constexpr int DEFAULT_CARD_W = 180;
static constexpr int MAX_CARD_HEIGHT = 840;
static constexpr int POSTER_RADIUS = 10;
static constexpr int CARD_PAD = 12;
static constexpr int TITLE_H = 44;

static QString cardStyleSheet()
{
	return QStringLiteral(
	           "#rankCard {"
	           "    background: transparent;"
	           "    border: 2px solid transparent;"
	           "    border-radius: 12px;"
	           "}"
	           "#rankCard:hover {"
	           "    border: 2px solid %1;"
	           "    background-color: %2;"
	           "}"
	)
	    .arg(Palette::accent, Palette::surface);
}

class RankingCard : public QWidget
{
  public:
	explicit RankingCard(QWidget *parent = nullptr) : QWidget(parent)
	{
		setAttribute(Qt::WA_StyledBackground, true);
		setAttribute(Qt::WA_Hover, true);
		setObjectName("rankCard");
		setCursor(Qt::PointingHandCursor);

		titleLabel = new ElidedLabel({}, 2, this);
		titleLabel->setAlignment(Qt::AlignCenter);

		setStyleSheet(cardStyleSheet());
		rescale(DEFAULT_CARD_W);
	}

	void setTitle(const Title &title)
	{
		sourcePixmap = title.posterImage;
		titleLabel->setText(title.title);
		updateScaledPixmap();
		update();
	}

	void rescale(int newCardW)
	{
		cardW = newCardW;
		posterH = cardW * 3 / 2;
		setFixedSize(
		    cardW + 2 * CARD_PAD,
		    CARD_PAD + posterH + CARD_PAD / 2 + TITLE_H + CARD_PAD / 2
		);
		titleLabel
		    ->setGeometry(CARD_PAD, CARD_PAD + posterH + CARD_PAD / 2, cardW, TITLE_H);
		applyLabelStyle();
		updateScaledPixmap();
		update();
	}

	void refreshStyle()
	{
		setStyleSheet(cardStyleSheet());
		applyLabelStyle();
		update();
	}

  protected:
	void paintEvent(QPaintEvent *) override
	{
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		QPainterPath clip;
		clip.addRoundedRect(
		    QRectF(CARD_PAD, CARD_PAD, cardW, posterH),
		    POSTER_RADIUS,
		    POSTER_RADIUS
		);
		p.setClipPath(clip);

		if(scaledPixmap.isNull())
			p.fillRect(CARD_PAD, CARD_PAD, cardW, posterH, QColor(Palette::surface));
		else
		{
			const int x = CARD_PAD + (cardW - scaledPixmap.width()) / 2;
			const int y = CARD_PAD + (posterH - scaledPixmap.height()) / 2;
			p.drawPixmap(x, y, scaledPixmap);
		}
	}

  private:
	void applyLabelStyle()
	{
		titleLabel->setStyleSheet(
		    QStringLiteral(
		        "border: none; background: transparent; color: %1; font-size: 12px;"
		    )
		        .arg(Palette::textPrimary)
		);
	}

	void updateScaledPixmap()
	{
		if(sourcePixmap.isNull() || cardW == 0)
			return;
		scaledPixmap = sourcePixmap.scaled(
		    QSize(cardW, posterH),
		    Qt::KeepAspectRatioByExpanding,
		    Qt::SmoothTransformation
		);
	}

	ElidedLabel *titleLabel = nullptr;
	QPixmap      sourcePixmap;
	QPixmap      scaledPixmap;
	int          cardW = 0;
	int          posterH = 0;
};

RankingView::RankingView(AppStorage &appStorage, QWidget *parent)
    : QWidget(parent), appStorage(appStorage)
{
	auto guard = appStorage.lock();
	for(const Title &t : appStorage.getTitles(guard))
	{
		if(t.rank != 0 || !t.lastViewed.isValid())
			continue;
		if(t.type == "movie")
			unrankedMovies.push_back(t);
		else if(t.type == "series")
			unrankedTvShows.push_back(t);
	}

	totalMovies = static_cast<int>(unrankedMovies.size());
	totalTvShows = static_cast<int>(unrankedTvShows.size());

	setupUi();
}

void RankingView::start()
{
	startNextTitle();
}

void RankingView::setupUi()
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	setAttribute(Qt::WA_StyledBackground, true);

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->setSpacing(0);

	auto *topRow = new QWidget;
	auto *topLayout = new QHBoxLayout(topRow);
	topLayout->setContentsMargins(0, 0, 0, 0);

	exitBtn = new IconButton(
	    AssetsPaths::crossIcon,
	    32,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	connect(exitBtn, &QPushButton::clicked, this, &RankingView::finished);

	progressLabel = new QLabel;
	progressLabel->setStyleSheet(
	    QStringLiteral("color: %1; font-size: 14px;").arg(Palette::textSecondary)
	);

	topLayout->addWidget(exitBtn);
	topLayout->addStretch();
	topLayout->addWidget(progressLabel);

	instructionLabel = new QLabel("Which do you prefer?");
	instructionLabel->setAlignment(Qt::AlignCenter);
	QFont instrFont;
	instrFont.setPixelSize(22);
	instrFont.setBold(true);
	instructionLabel->setFont(instrFont);
	instructionLabel->setStyleSheet(
	    QStringLiteral("color: %1;").arg(Palette::textPrimary)
	);

	cardsRow = new QWidget;
	auto *cardsLayout = new QHBoxLayout(cardsRow);
	cardsLayout->setContentsMargins(0, 0, 0, 0);
	cardsLayout->setSpacing(48);
	cardsLayout->setAlignment(Qt::AlignCenter);

	leftCard = new RankingCard(this);
	rightCard = new RankingCard(this);
	leftCard->installEventFilter(this);
	rightCard->installEventFilter(this);

	vsLabel = new QLabel("VS");
	vsLabel->setAlignment(Qt::AlignCenter);
	QFont vsFont;
	vsFont.setPixelSize(28);
	vsFont.setBold(true);
	vsLabel->setFont(vsFont);
	vsLabel->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::textSecondary));

	cardsLayout->addWidget(leftCard);
	cardsLayout->addWidget(vsLabel);
	cardsLayout->addWidget(rightCard);

	mainLayout->addWidget(topRow);
	mainLayout->addStretch();
	mainLayout->addWidget(instructionLabel, 0, Qt::AlignCenter);
	mainLayout->addSpacing(36);
	mainLayout->addWidget(cardsRow, 0, Qt::AlignCenter);
	mainLayout->addStretch();

	updateCardSize();
}

void RankingView::refreshStyle()
{
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));
	exitBtn->updateColors(Palette::accent, Palette::surface);
	progressLabel->setStyleSheet(
	    QStringLiteral("color: %1; font-size: 14px;").arg(Palette::textSecondary)
	);
	instructionLabel->setStyleSheet(
	    QStringLiteral("color: %1;").arg(Palette::textPrimary)
	);
	vsLabel->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::textSecondary));
	leftCard->refreshStyle();
	rightCard->refreshStyle();
}

bool RankingView::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease)
	{
		if(obj == leftCard)
			onLeftChosen();
		else if(obj == rightCard)
			onRightChosen();
	}
	return QObject::eventFilter(obj, event);
}

void RankingView::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	updateCardSize();
}

void RankingView::updateCardSize()
{
	if(!leftCard)
		return;
	const int posterH = qMax(150, qMin(height() - 220, MAX_CARD_HEIGHT));
	const int cardW = posterH * 2 / 3;
	leftCard->rescale(cardW);
	rightCard->rescale(cardW);
	cardsRow->adjustSize();
}

std::vector<Title> RankingView::rankedTitlesOfType(const QString &type) const
{
	std::vector<Title> result;

	auto guard = appStorage.lock();
	for(const Title &t : appStorage.getTitles(guard))
		if(t.type == type && t.rank > 0)
			result.push_back(t);

	std::sort(
	    result.begin(),
	    result.end(),
	    [](const Title &a, const Title &b) { return a.rank < b.rank; }
	);

	return result;
}

void RankingView::updateProgress()
{
	const QString phase = processingTvShows ? "TV Shows" : "Movies";
	const int     total = processingTvShows ? totalTvShows : totalMovies;
	const int     done = processingTvShows ? processedTvShows : processedMovies;
	progressLabel->setText(
	    QStringLiteral("Ranking %1: %2 / %3").arg(phase).arg(done + 1).arg(total)
	);
}

void RankingView::startNextTitle()
{
	if(!processingTvShows && unrankedMovies.empty())
		processingTvShows = true;

	auto &queue = processingTvShows ? unrankedTvShows : unrankedMovies;

	if(queue.empty())
	{
		emit finished();
		return;
	}

	currentUnranked = queue.front();
	queue.erase(queue.begin());

	currentRanked = rankedTitlesOfType(currentUnranked.type);
	low = 0;
	high = static_cast<int>(currentRanked.size());

	updateProgress();

	if(high == 0)
		insertCurrent();
	else
		showComparison();
}

void RankingView::showComparison()
{
	const int mid = (low + high) / 2;
	leftCard->setTitle(currentUnranked);
	rightCard->setTitle(currentRanked[mid]);
}

void RankingView::onLeftChosen()
{
	high = (low + high) / 2;
	if(low < high)
		showComparison();
	else
		insertCurrent();
}

void RankingView::onRightChosen()
{
	low = (low + high) / 2 + 1;
	if(low < high)
		showComparison();
	else
		insertCurrent();
}

void RankingView::insertCurrent()
{
	appStorage.insertRank(currentUnranked.imdbId, low, currentUnranked.type);

	if(processingTvShows)
		processedTvShows++;
	else
		processedMovies++;

	startNextTitle();
}
