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
	AppStorage &appStorage;
	QVector<Title *> titles;
	bool queueEmpty = false;

	bool isEligible(const Title &t) const;
};
