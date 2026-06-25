// Owns the library-update lifecycle: runs LibraryUpdate on a background thread
// and retries on transient network failures.
#pragma once

#include "AppStorage.hpp"

#include <QObject>
#include <QTimer>

class LibraryUpdate;

class LibraryUpdateController : public QObject
{
	Q_OBJECT

  public:
	explicit LibraryUpdateController(AppStorage &appStorage, QObject *parent = nullptr);

	void start();

  signals:
	void updateStarted();
	void updateFinished();
	void updateFailed(const QString &message);

  private:
	AppStorage &appStorage;
	QTimer     *retryTimer;
	bool        running = false;

	void checkConnectivityAndRetry();
	void runAttempt(LibraryUpdate *queue);
	void handleAttemptResult(const QString &errorMessage, bool isNetworkError);
};
