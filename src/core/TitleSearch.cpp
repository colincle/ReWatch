// fts_fuzzy_match scores each title field; Levenshtein is the fallback for typos that
// fuzzy misses. sortKey strips leading articles so "The Wire" sorts under W.
#include "TitleSearch.hpp"
#include "Levenshtein.hpp"

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.h"

#include <algorithm>
#include <QDate>

static int fuzzyScore(const QString &pattern, const QString &str)
{
	int              score = 0;
	const QByteArray p = pattern.toLower().toUtf8();
	const QByteArray s = str.toLower().toUtf8();
	fts::fuzzy_match(p.constData(), s.constData(), score);
	return score;
}

static bool titleLevenshteinMatches(const Title &t, const QString &query)
{
	return levenshteinMatches(query, t.title) || levenshteinMatches(query, t.actors) ||
	       levenshteinMatches(query, t.director) || levenshteinMatches(query, t.released);
}

static int titleBestScore(const Title &t, const QString &query)
{
	return std::max({
	    fuzzyScore(query, t.title),
	    fuzzyScore(query, t.actors),
	    fuzzyScore(query, t.director),
	    fuzzyScore(query, t.released),
	});
}

std::vector<Title>
scoreAndRankTitles(const std::vector<Title> &titles, const QString &query)
{
	if(query.isEmpty())
	{
		return titles;
	}

	std::vector<std::pair<int, Title>> scored;

	for(const Title &t : titles)
	{
		int score = titleBestScore(t, query);

		if(score > 0)
		{
			scored.emplace_back(score, t);
		}
		else if(titleLevenshteinMatches(t, query))
		{
			scored.emplace_back(1, t);
		}
	}

	std::sort(
	    scored.begin(),
	    scored.end(),
	    [](const auto &a, const auto &b) { return a.first > b.first; }
	);

	std::vector<Title> result;

	for(auto &[score, t] : scored)
	{
		result.push_back(std::move(t));
	}

	return result;
}

std::vector<Title>
filterTitles(const std::vector<Title> &titles, LibraryTab tab, ViewFilter filter)
{
	std::vector<Title> result;

	for(const Title &t : titles)
	{
		if(filter == ViewFilter::ToWatch && t.viewed)
		{
			continue;
		}

		if(tab == LibraryTab::Movies && t.type == "movie")
		{
			result.push_back(t);
		}

		if(tab == LibraryTab::TvShows && t.type == "series")
		{
			result.push_back(t);
		}
	}

	return result;
}

static QString sortKey(const QString &title)
{
	QString s = title.toLower();

	if(s.startsWith("the "))
	{
		return s.sliced(4);
	}

	if(s.startsWith("an "))
	{
		return s.sliced(3);
	}

	if(s.startsWith("a "))
	{
		return s.sliced(2);
	}

	return s;
}

void sortTitles(std::vector<Title> &titles, SortMode mode)
{
	switch(mode)
	{
	case SortMode::AlphaAZ:
		std::sort(
		    titles.begin(),
		    titles.end(),
		    [](const Title &a, const Title &b)
		    { return sortKey(a.title) < sortKey(b.title); }
		);
		break;

	case SortMode::Release:
		std::sort(
		    titles.begin(),
		    titles.end(),
		    [](const Title &a, const Title &b)
		    {
			    return QDate::fromString(a.released, "dd MMM yyyy") >
			           QDate::fromString(b.released, "dd MMM yyyy");
		    }
		);
		break;

	case SortMode::WatchDate:
		std::sort(
		    titles.begin(),
		    titles.end(),
		    [](const Title &a, const Title &b)
		    {
			    if(!a.lastViewed.isValid() && !b.lastViewed.isValid())
				    return false;
			    if(!a.lastViewed.isValid())
				    return true;
			    if(!b.lastViewed.isValid())
				    return false;
			    return a.lastViewed < b.lastViewed;
		    }
		);
		break;

	case SortMode::Rank:
		titles.erase(
		    std::remove_if(
		        titles.begin(),
		        titles.end(),
		        [](const Title &t) { return t.rank == 0; }
		    ),
		    titles.end()
		);
		std::sort(
		    titles.begin(),
		    titles.end(),
		    [](const Title &a, const Title &b) { return a.rank < b.rank; }
		);
		break;
	}
}
