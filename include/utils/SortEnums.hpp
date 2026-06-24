// Enums shared across the library view, top bar, and title search pipeline.
#pragma once

enum class SortMode
{
	AlphaAZ,
	Rank,
	WatchDate,
	Release
};

enum class LibraryTab
{
	Movies,
	TvShows
};

enum class ViewFilter
{
	All,
	ToWatch
};