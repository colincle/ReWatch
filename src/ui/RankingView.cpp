#include "RankingView.hpp"
#include "AssetsPaths.hpp"
#include "IconButton.hpp"
#include "Palette.hpp"

#include <algorithm>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>

static constexpr int POSTER_W = 180;
static constexpr int POSTER_H = 270;

RankingView::RankingView(AppStorage &appStorage, QWidget *parent)
    : QWidget(parent), appStorage(appStorage)
{
	for(const Title &t : appStorage.getTitles())
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
	mainLayout->setContentsMargins(40, 24, 40, 40);
	mainLayout->setSpacing(0);

	auto *topRow = new QWidget;
	auto *topLayout = new QHBoxLayout(topRow);
	topLayout->setContentsMargins(0, 0, 0, 0);

	auto *exitBtn = new IconButton(
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

	auto *instructionLabel = new QLabel("Which do you prefer?");
	instructionLabel->setAlignment(Qt::AlignCenter);
	QFont instrFont;
	instrFont.setPixelSize(22);
	instrFont.setBold(true);
	instructionLabel->setFont(instrFont);
	instructionLabel->setStyleSheet(
	    QStringLiteral("color: %1;").arg(Palette::textPrimary)
	);

	auto *cardsRow = new QWidget;
	auto *cardsLayout = new QHBoxLayout(cardsRow);
	cardsLayout->setContentsMargins(0, 0, 0, 0);
	cardsLayout->setSpacing(48);
	cardsLayout->setAlignment(Qt::AlignCenter);

	leftCard = makeCard(leftPosterLabel, leftTitleLabel);
	rightCard = makeCard(rightPosterLabel, rightTitleLabel);

	leftCard->installEventFilter(this);
	rightCard->installEventFilter(this);

	auto *vsLabel = new QLabel("VS");
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
}

QWidget *RankingView::makeCard(QLabel *&posterOut, QLabel *&titleOut)
{
	auto *card = new QWidget;
	card->setObjectName("rankCard");
	card->setAttribute(Qt::WA_StyledBackground, true);
	card->setAttribute(Qt::WA_Hover, true);
	card->setStyleSheet(QStringLiteral(
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
	                        .arg(Palette::accent, Palette::surface));

	auto *layout = new QVBoxLayout(card);
	layout->setContentsMargins(12, 12, 12, 16);
	layout->setSpacing(12);

	posterOut = new QLabel;
	posterOut->setFixedSize(POSTER_W, POSTER_H);
	posterOut->setAlignment(Qt::AlignCenter);
	posterOut->setStyleSheet("background: transparent; border: none;");

	titleOut = new QLabel;
	titleOut->setAlignment(Qt::AlignCenter);
	titleOut->setWordWrap(true);
	titleOut->setFixedWidth(POSTER_W);
	QFont titleFont;
	titleFont.setPixelSize(14);
	titleFont.setBold(true);
	titleOut->setFont(titleFont);
	titleOut->setStyleSheet(
	    QStringLiteral("color: %1; background: transparent; border: none;")
	        .arg(Palette::textPrimary)
	);

	layout->addWidget(posterOut);
	layout->addWidget(titleOut, 0, Qt::AlignCenter);

	return card;
}

void RankingView::populateCard(
    QLabel *posterLabel, QLabel *titleLabel, const Title &title
)
{
	posterLabel->setPixmap(title.posterImage.scaled(
	    posterLabel->size(),
	    Qt::KeepAspectRatio,
	    Qt::SmoothTransformation
	));
	titleLabel->setText(title.title);
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

std::vector<Title> RankingView::rankedTitlesOfType(const QString &type) const
{
	std::vector<Title> result;

	for(const Title &t : appStorage.getTitles())
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
	{
		insertCurrent();
	}
	else
	{
		showComparison();
	}
}

void RankingView::showComparison()
{
	const int mid = (low + high) / 2;
	populateCard(leftPosterLabel, leftTitleLabel, currentUnranked);
	populateCard(rightPosterLabel, rightTitleLabel, currentRanked[mid]);
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
