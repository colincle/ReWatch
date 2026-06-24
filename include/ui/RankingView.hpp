// Binary-search ranking UI. Presents pairs of titles for the user to compare; inserts
// each new title into its correct position with O(log n) comparisons.
#pragma once

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "Title.hpp"

#include <QLabel>
#include <QWidget>
#include <vector>

class QResizeEvent;
class QVBoxLayout;
class RankingCard;

class RankingView : public QWidget
{
	Q_OBJECT

  public:
	explicit RankingView(AppStorage &appStorage, QWidget *parent = nullptr);
	void start();
	void refreshStyle();

  signals:
	void finished();

  protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

  private:
	AppStorage &appStorage;

	std::vector<Title> unrankedMovies;
	std::vector<Title> unrankedTvShows;
	bool               processingTvShows = false;

	Title              currentUnranked;
	std::vector<Title> currentRanked;
	int                low = 0;
	int                high = 0;

	int totalMovies = 0;
	int totalTvShows = 0;
	int processedMovies = 0;
	int processedTvShows = 0;

	QWidget     *cardsRow = nullptr;
	RankingCard *leftCard = nullptr;
	RankingCard *rightCard = nullptr;
	QLabel      *progressLabel = nullptr;
	QLabel      *instructionLabel = nullptr;
	QLabel      *vsLabel = nullptr;
	IconButton  *exitBtn = nullptr;

	void setupUi();
	void startNextTitle();
	void showComparison();
	void onLeftChosen();
	void onRightChosen();
	void insertCurrent();
	void updateCardSize();

	std::vector<Title> rankedTitlesOfType(const QString &type) const;
	void               updateProgress();
};
