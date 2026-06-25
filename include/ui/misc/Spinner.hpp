// Animated loading indicator that slides three blocks in a 2x2 grid (sliding puzzle).
#pragma once

#include <QString>
#include <QTimer>
#include <QWidget>

class Spinner : public QWidget
{
	Q_OBJECT

  public:
	explicit Spinner(QString color, int speed, QWidget *parent = nullptr);
	void setColor(const QString &c)
	{
		color = c;
		update();
	}

  protected:
	void paintEvent(QPaintEvent *) override;
	void showEvent(QShowEvent *) override;
	void hideEvent(QHideEvent *) override;

  private:
	QTimer  timer;
	QString color;

	int blockCell[3] = {0, 1, 2};
	int emptyCell = 3;
	int movingBlock = -1;
	int movingFrom = -1;
	int movingTo = -1;

	double progress = 0.0;

	void prepareNextMove();
	void onTick(double progressPerFrame);
};