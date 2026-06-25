// Constructs the priority-ordered, budget-capped update queue and runs all OMDb fetches
// synchronously in run(), which is designed to run on a background thread.
#include "LibraryUpdate.hpp"
#include "OmdbSearch.hpp"

#include <algorithm>
#include <QDate>
#include <QEventLoop>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace
{

struct TitleFetchResult
{
	bool        success = false;
	bool        isAuthFailure = false;
	bool        isNetworkFailure = false;
	QJsonObject data;
};

struct EpisodeResult
{
	bool        authFailure = false;
	bool        networkFailure = false;
	LastEpisode le;
	QString     nextSeasonDate;
	int         requestsMade = 0;
};

TitleFetchResult parseReply(QNetworkReply *reply)
{
	TitleFetchResult result;

	if(reply->error() != QNetworkReply::NoError)
	{
		const int status =
		    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		const QString message = reply->errorString();
		if(status == 401 || OmdbSearch::isAuthError(message))
			result.isAuthFailure = true;
		else
			result.isNetworkFailure = true;
		return result;
	}

	result.data = QJsonDocument::fromJson(reply->readAll()).object();

	if(result.data["Response"].toString() == "False")
	{
		result.isAuthFailure = OmdbSearch::isAuthError(result.data["Error"].toString());
		return result;
	}

	result.success = true;
	return result;
}

// Eligible = release date arrived AND not already checked today.
QSet<QString> buildUpcomingEligibleSet(
    AppStorage &appStorage, AppStorage::LockGuard &lock, const QDate &today
)
{
	QSet<QString> eligible;
	const auto   &titles = appStorage.getTitles(lock);

	for(const QString &id : appStorage.getUpcomingMovies())
	{
		auto it = std::find_if(
		    titles.begin(),
		    titles.end(),
		    [&](const Title &t) { return t.imdbId == id; }
		);
		if(it == titles.end())
			continue;
		const QDate releaseDate = QDate::fromString(it->released, "dd MMM yyyy");
		if(releaseDate.isValid() && releaseDate > today)
			continue;
		if(it->lastChecked == today)
			continue;
		eligible.insert(id);
	}
	return eligible;
}

// Order: priority list → remaining series → remaining upcoming movies.
QVector<QString> buildOrderedQueue(
    const std::vector<QString> &priority, const std::vector<Title> &titles,
    const std::vector<QString> &upcomingMovies, const QSet<QString> &prioritySet,
    const QSet<QString> &eligibleSeries, const QSet<QString> &eligibleUpcoming
)
{
	QVector<QString> ordered;

	for(const QString &id : priority)
		if(eligibleSeries.contains(id) || eligibleUpcoming.contains(id))
			ordered.push_back(id);

	for(const Title &t : titles)
		if(eligibleSeries.contains(t.imdbId) && !prioritySet.contains(t.imdbId))
			ordered.push_back(t.imdbId);

	for(const QString &id : upcomingMovies)
		if(eligibleUpcoming.contains(id) && !prioritySet.contains(id))
			ordered.push_back(id);

	return ordered;
}

// All requests are fired concurrently; the event loop blocks until the last one lands.
QVector<TitleFetchResult> fetchTitles(
    const QVector<QString> &imdbIds, QNetworkAccessManager &manager, const QString &apiKey
)
{
	const int                 count = imdbIds.size();
	QVector<TitleFetchResult> results(count);
	int                       pending = count;
	QEventLoop                loop;

	for(int i = 0; i < count; ++i)
	{
		QNetworkReply *reply =
		    manager.get(QNetworkRequest(OmdbSearch::makeUrl(apiKey, "i", imdbIds[i])));

		QObject::connect(
		    reply,
		    &QNetworkReply::finished,
		    &loop,
		    [&, i, reply]()
		    {
			    results[i] = parseReply(reply);
			    reply->deleteLater();
			    if(--pending == 0)
				    loop.quit();
		    }
		);
	}

	if(pending > 0)
		loop.exec();

	return results;
}

// Sequential (unlike fetchTitles) because each series may need several season API calls.
QVector<EpisodeResult> fetchEpisodes(
    const QVector<QString> &imdbIds, const QVector<int> &storedSeasons,
    const QSet<QString> &upcomingMovieIds, const QVector<TitleFetchResult> &results,
    QNetworkAccessManager &manager, const QString &apiKey
)
{
	const int              count = imdbIds.size();
	QVector<EpisodeResult> episodeResults(count);

	for(int i = 0; i < count; ++i)
	{
		if(!results[i].success || upcomingMovieIds.contains(imdbIds[i]))
			continue;

		const int apiSeason = results[i].data["totalSeasons"].toString().toInt();
		episodeResults[i].le.season = std::max({apiSeason, storedSeasons[i], 1});

		const QDate showRelease =
		    QDate::fromString(results[i].data["Released"].toString(), "dd MMM yyyy");

		const SearchErrorType err = OmdbSearch::findLastEpisode(
		    manager,
		    apiKey,
		    imdbIds[i],
		    episodeResults[i].le,
		    episodeResults[i].nextSeasonDate,
		    showRelease,
		    &episodeResults[i].requestsMade
		);

		if(err == SearchErrorType::AuthInvalid)
			episodeResults[i].authFailure = true;
		else if(err != SearchErrorType::None)
			episodeResults[i].networkFailure = true;
	}

	return episodeResults;
}

void applyMovieResult(
    Title &title, const QJsonObject &data, std::vector<Notification> &notifications,
    std::vector<QString> &moviesToRelease
)
{
	const QString newReleased = data["Released"].toString();
	const QDate   newDate = QDate::fromString(newReleased, "dd MMM yyyy");

	if(!newReleased.isEmpty() && newReleased != "N/A")
		title.released = newReleased;

	if(newDate.isValid() && newDate > QDate::currentDate())
		return; // date pushed back — keep in upcoming with updated date

	moviesToRelease.push_back(title.imdbId);
	notifications.push_back({title.imdbId, NotificationType::MovieRelease});
}

void applySeriesResult(
    Title &title, const EpisodeResult &er, std::vector<Notification> &notifications
)
{
	const LastEpisode oldLE = title.lastEpisode;
	title.nextSeasonDate = er.nextSeasonDate;
	title.lastEpisode = er.le;

	if(oldLE.season == 0)
		return; // no prior baseline, skip notification

	if(er.le.season > oldLE.season)
	{
		title.viewed = false;
		notifications.push_back({title.imdbId, NotificationType::NewSeason});
	}
	else if(er.le.season == oldLE.season && er.le.episode > oldLE.episode)
	{
		title.viewed = false;
		notifications.push_back({title.imdbId, NotificationType::NewEpisode});
	}
}

} // namespace

LibraryUpdate::LibraryUpdate(AppStorage &appStorage, QObject *parent)
    : QObject(parent), appStorage(appStorage)
{
	auto lock = appStorage.lock();

	const QDate today = QDate::currentDate();
	const int   usedToday =
	    (appStorage.getChecksDate() == today) ? appStorage.getChecksToday() : 0;
	const int maxRequests = std::max(0, appStorage.getMaxUpdateRequests() - usedToday);

	const auto   &priority = appStorage.getUpdatePriority(lock);
	QSet<QString> prioritySet(priority.begin(), priority.end());

	QSet<QString> eligibleSeries;
	for(const Title &t : appStorage.getTitles(lock))
		if(isEligible(t))
			eligibleSeries.insert(t.imdbId);

	const QSet<QString> eligibleUpcoming =
	    buildUpcomingEligibleSet(appStorage, lock, today);

	const QVector<QString> ordered = buildOrderedQueue(
	    priority,
	    appStorage.getTitles(lock),
	    appStorage.getUpcomingMovies(),
	    prioritySet,
	    eligibleSeries,
	    eligibleUpcoming
	);

	const int take = std::min(static_cast<int>(ordered.size()), maxRequests);
	imdbIds = ordered.mid(0, take);

	const auto &titles = appStorage.getTitles(lock);
	for(const QString &id : imdbIds)
	{
		auto it = std::find_if(
		    titles.begin(),
		    titles.end(),
		    [&](const Title &t) { return t.imdbId == id; }
		);
		storedSeasons.push_back(it != titles.end() ? it->lastEpisode.season : 1);
		if(eligibleUpcoming.contains(id))
			upcomingMovieIds.insert(id);
	}

	appStorage.addUpdateChecks(take);
	appStorage.setUpdatePriority({ordered.begin() + take, ordered.end()});

	if(imdbIds.isEmpty())
		queueEmpty = true;
}

bool LibraryUpdate::isEligible(const Title &t) const
{
	if(t.type != "series")
		return false;
	if(!t.lastChecked.isValid())
		return true;
	return t.lastChecked.daysTo(QDate::currentDate()) >= 1;
}

void LibraryUpdate::run()
{
	const QString         apiKey = appStorage.getKey();
	QNetworkAccessManager manager;

	const auto results = fetchTitles(imdbIds, manager, apiKey);
	const auto episodeResults =
	    fetchEpisodes(imdbIds, storedSeasons, upcomingMovieIds, results, manager, apiKey);

	int totalEpisodeRequests = 0;
	for(const EpisodeResult &er : episodeResults)
		totalEpisodeRequests += er.requestsMade;
	if(totalEpisodeRequests > 0)
		appStorage.addUpdateChecks(totalEpisodeRequests);

	std::vector<Notification> notifications;
	std::vector<QString>      moviesToRelease;
	bool                      hasAuthError = false;
	bool                      hasNetworkError = false;

	{
		auto  lock = appStorage.lock();
		auto &titles = appStorage.getTitlesMutable(lock);

		for(int i = 0; i < imdbIds.size(); ++i)
		{
			if(results[i].isAuthFailure || episodeResults[i].authFailure)
			{
				hasAuthError = true;
				continue;
			}
			if(results[i].isNetworkFailure || episodeResults[i].networkFailure)
			{
				hasNetworkError = true;
				continue;
			}
			if(!results[i].success)
				continue;

			auto it = std::find_if(
			    titles.begin(),
			    titles.end(),
			    [&](const Title &t) { return t.imdbId == imdbIds[i]; }
			);
			if(it == titles.end())
				continue;

			it->lastChecked = QDate::currentDate();

			if(upcomingMovieIds.contains(imdbIds[i]))
				applyMovieResult(*it, results[i].data, notifications, moviesToRelease);
			else
				applySeriesResult(*it, episodeResults[i], notifications);
		}
	}

	appStorage.removeFromUpcomingMovies(moviesToRelease);
	appStorage.addNotifications(notifications);
	emit finished();

	if(hasAuthError)
		emit apiKeyError();
	else if(hasNetworkError)
		emit networkError();
}
