#pragma once

#include "AppStorage.hpp"
#include "Title.hpp"

#include <QLabel>
#include <QWidget>
#include <vector>

class QVBoxLayout;

class RankingView : public QWidget
{
	Q_OBJECT

  public:
	explicit RankingView(AppStorage &appStorage, QWidget *parent = nullptr);
	void start();

  signals:
	void finished();

  protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

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

	QWidget *leftCard = nullptr;
	QWidget *rightCard = nullptr;
	QLabel  *leftPosterLabel = nullptr;
	QLabel  *rightPosterLabel = nullptr;
	QLabel  *leftTitleLabel = nullptr;
	QLabel  *rightTitleLabel = nullptr;
	QLabel  *progressLabel = nullptr;

	void     setupUi();
	QWidget *makeCard(QLabel *&posterOut, QLabel *&titleOut);
	void     populateCard(QLabel *posterLabel, QLabel *titleLabel, const Title &title);

	void startNextTitle();
	void showComparison();
	void onLeftChosen();
	void onRightChosen();
	void insertCurrent();

	std::vector<Title> rankedTitlesOfType(const QString &type) const;
	void               updateProgress();
};
