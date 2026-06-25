// Drives the library-update lifecycle. Runs LibraryUpdate on a QtConcurrent thread and
// retries network failures every UPDATE_RETRY_INTERVAL_MS until connectivity is restored.
#include "LibraryUpdateController.hpp"
#include "ErrorMessages.hpp"
#include "LibraryUpdate.hpp"

#include <QEventLoop>
#include <QFutureWatcher>
#include <QNetworkInformation>
#include <QtConcurrent>

static constexpr int UPDATE_RETRY_INTERVAL_MS = 5000;

LibraryUpdateController::LibraryUpdateController(AppStorage &appStorage, QObject *parent)
    : QObject(parent), appStorage(appStorage)
{
	QNetworkInformation::loadDefaultBackend();

	retryTimer = new QTimer(this);
	retryTimer->setInterval(UPDATE_RETRY_INTERVAL_MS);
	connect(
	    retryTimer,
	    &QTimer::timeout,
	    this,
	    &LibraryUpdateController::checkConnectivityAndRetry
	);
}

void LibraryUpdateController::checkConnectivityAndRetry()
{
	auto *networkInfo = QNetworkInformation::instance();

	if(networkInfo &&
	   networkInfo->reachability() != QNetworkInformation::Reachability::Online)
	{
		return;
	}

	start();
}

void LibraryUpdateController::start()
{
	if(running)
		return;

	auto *queue = new LibraryUpdate(appStorage, this);

	if(queue->isEmpty())
	{
		delete queue;
		retryTimer->stop();
		return;
	}

	running = true;
	QTimer::singleShot(0, this, [this, queue]() { runAttempt(queue); });
}

void LibraryUpdateController::runAttempt(LibraryUpdate *queue)
{
	emit updateStarted();

	QString errorMessage;
	bool    isNetworkError = false;

	connect(
	    queue,
	    &LibraryUpdate::apiKeyError,
	    this,
	    [&errorMessage, &isNetworkError]()
	    {
		    errorMessage = API_KEY_ERROR_MESSAGE;
		    isNetworkError = false;
	    },
	    Qt::QueuedConnection
	);

	connect(
	    queue,
	    &LibraryUpdate::networkError,
	    this,
	    [&errorMessage, &isNetworkError]()
	    {
		    errorMessage = UPDATE_NETWORK_ERROR_MESSAGE;
		    isNetworkError = true;
	    },
	    Qt::QueuedConnection
	);

	connect(
	    queue,
	    &LibraryUpdate::finished,
	    &appStorage,
	    &AppStorage::titlesUpdated,
	    Qt::QueuedConnection
	);

	QEventLoop loop;
	auto       future = QtConcurrent::run([queue]() { queue->run(); });

	QFutureWatcher<void> watcher;
	connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
	watcher.setFuture(future);
	loop.exec();

	queue->deleteLater();
	running = false;
	emit updateFinished();
	handleAttemptResult(errorMessage, isNetworkError);
}

void LibraryUpdateController::handleAttemptResult(
    const QString &errorMessage, bool isNetworkError
)
{
	if(errorMessage.isEmpty())
	{
		retryTimer->stop();
		return;
	}

	if(!isNetworkError)
	{
		retryTimer->stop();
		emit updateFailed(errorMessage);
		return;
	}

	if(!retryTimer->isActive())
	{
		emit updateFailed(errorMessage);
	}

	retryTimer->start();
}
