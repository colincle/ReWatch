// Builds the per-launch update queue (priority-ordered, daily-budget-capped) and
// runs all OMDb fetches synchronously on whatever thread calls run().
#pragma once

#include "AppStorage.hpp"

#include <QObject>
#include <QSet>
#include <QVector>

class LibraryUpdate : public QObject
{
	Q_OBJECT

  public:
	explicit LibraryUpdate(AppStorage &appStorage, QObject *parent = nullptr);

	bool isEmpty() const { return queueEmpty; }
	void run();

  signals:
	void apiKeyError();
	void networkError();
	void finished();

  private:
	AppStorage      &appStorage;
	QVector<QString> imdbIds;
	QVector<int>     storedSeasons;
	QSet<QString>    upcomingMovieIds;
	bool             queueEmpty = false;

	bool isEligible(const Title &t) const;
};
