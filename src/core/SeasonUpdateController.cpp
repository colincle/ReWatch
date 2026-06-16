#include "SeasonUpdateController.hpp"
#include "ErrorMessages.hpp"
#include "SeasonUpdate.hpp"

#include <QElapsedTimer>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QNetworkInformation>
#include <QtConcurrent>

static constexpr int SEASON_RETRY_INTERVAL_MS = 5000;
static constexpr int MIN_DISPLAY_MS = 1000;

static const QString NETWORK_ERROR_MESSAGE =
    "Couldn't check for new seasons — check your internet connection.";

SeasonUpdateController::SeasonUpdateController(AppStorage &appStorage, QObject *parent)
	: QObject(parent)
	, appStorage(appStorage)
{
	QNetworkInformation::loadDefaultBackend();

	retryTimer = new QTimer(this);
	retryTimer->setInterval(SEASON_RETRY_INTERVAL_MS);
	connect(retryTimer, &QTimer::timeout, this, &SeasonUpdateController::checkConnectivityAndRetry);
}

void SeasonUpdateController::checkConnectivityAndRetry()
{
	auto *networkInfo = QNetworkInformation::instance();

	if(networkInfo && networkInfo->reachability() != QNetworkInformation::Reachability::Online)
	{
		return;
	}

	start();
}

void SeasonUpdateController::start()
{
	auto *queue = new SeasonUpdate(appStorage, this);

	if(queue->isEmpty())
	{
		retryTimer->stop();
		return;
	}

	QTimer::singleShot(0, this, [this, queue]()
	{
		runAttempt(queue);
	});
}

void SeasonUpdateController::runAttempt(SeasonUpdate *queue)
{
	emit updateStarted();

	QElapsedTimer elapsed;
	elapsed.start();

	QString errorMessage;
	bool isNetworkError = false;

	connect(queue, &SeasonUpdate::apiKeyError, this, [&errorMessage, &isNetworkError]()
	{
		errorMessage = API_KEY_ERROR_MESSAGE;
		isNetworkError = false;
	}, Qt::QueuedConnection);

	connect(queue, &SeasonUpdate::networkError, this, [&errorMessage, &isNetworkError]()
	{
		errorMessage = NETWORK_ERROR_MESSAGE;
		isNetworkError = true;
	}, Qt::QueuedConnection);

	connect(queue, &SeasonUpdate::seriesUpdated, &appStorage, &AppStorage::titlesUpdated, Qt::QueuedConnection);

	QEventLoop loop;
	auto future = QtConcurrent::run([queue]() { queue->updateSeries(); });

	QFutureWatcher<void> watcher;
	connect(&watcher, &QFutureWatcher<void>::finished, &loop, [&]()
	{
		const int remaining = MIN_DISPLAY_MS - static_cast<int>(elapsed.elapsed());

		if(remaining > 0)
		{
			QTimer::singleShot(remaining, &loop, &QEventLoop::quit);
		}
		else
		{
			loop.quit();
		}
	});
	watcher.setFuture(future);
	loop.exec();

	emit updateFinished();
	handleAttemptResult(errorMessage, isNetworkError);
}

void SeasonUpdateController::handleAttemptResult(const QString &errorMessage, bool isNetworkError)
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
