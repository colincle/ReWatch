#pragma once

#include <QString>
#include <QStringList>

#include <numeric>
#include <vector>

inline int levenshtein(const QString &a, const QString &b)
{
	const int        m = a.size(), n = b.size();
	std::vector<int> row(n + 1);
	std::iota(row.begin(), row.end(), 0);

	for(int i = 1; i <= m; ++i)
	{
		int prev = i;

		for(int j = 1; j <= n; ++j)
		{
			int next = (a[i - 1] == b[j - 1]) ? row[j - 1]
			                                  : 1 + std::min({row[j - 1], row[j], prev});
			row[j - 1] = prev;
			prev = next;
		}

		row[n] = prev;
	}

	return row[n];
}

inline int levenshteinThreshold(const QString &query)
{
	const int len = query.size();

	if(len <= 4)
	{
		return 0;
	}

	if(len <= 7)
	{
		return 1;
	}

	return 2;
}

inline bool levenshteinMatches(const QString &query, const QString &field)
{
	const int threshold = levenshteinThreshold(query);

	if(threshold == 0)
	{
		return false;
	}

	const QString q = query.toLower();

	for(const QString &word : field.toLower().split(' ', Qt::SkipEmptyParts))
	{
		if(levenshtein(q, word) <= threshold)
		{
			return true;
		}
	}

	return false;
}
