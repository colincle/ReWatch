// Owns the season-update lifecycle: runs SeasonUpdate on a background thread,
// enforces a minimum display duration, and retries on transient network failures.
#pragma once

#include "AppStorage.hpp"

#include <QObject>
#include <QTimer>

class SeasonUpdate;

class SeasonUpdateController : public QObject
{
	Q_OBJECT

  public:
	explicit SeasonUpdateController(AppStorage &appStorage, QObject *parent = nullptr);

	void start();

  signals:
	void updateStarted();
	void updateFinished();
	void updateFailed(const QString &message);

  private:
	AppStorage &appStorage;
	QTimer     *retryTimer;

	void checkConnectivityAndRetry();
	void runAttempt(SeasonUpdate *queue);
	void handleAttemptResult(const QString &errorMessage, bool isNetworkError);
};
