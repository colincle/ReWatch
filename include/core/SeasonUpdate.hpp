// Builds the per-launch season-check queue (priority-ordered, daily-budget-capped) and
// runs all OMDb fetches synchronously on whatever thread calls updateSeries().
#pragma once

#include "AppStorage.hpp"

#include <QObject>
#include <QVector>

class SeasonUpdate : public QObject
{
	Q_OBJECT

  public:
	explicit SeasonUpdate(AppStorage &appStorage, QObject *parent = nullptr);

	bool isEmpty() const { return queueEmpty; }
	void updateSeries();

  signals:
	void apiKeyError();
	void networkError();
	void seriesUpdated();

  private:
	AppStorage      &appStorage;
	QVector<QString> imdbIds;
	bool             queueEmpty = false;

	bool isEligible(const Title &t) const;
};
