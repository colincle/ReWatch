// All help dialog content in one place. Add or edit items here; HelpDialog renders them.
#pragma once

#include <QString>
#include <QVector>

struct HelpItem
{
	QString name;
	QString description;
};

inline QVector<HelpItem> helpItems()
{
	return {
	    {
	        "Notifications and new seasons",
	        "Click the bell icon in the top bar to open the notification panel. "
	        "A red dot appears on the bell automatically when a new season of a TV show "
	        "in your library is detected - no need to check manually. "
	        "Click any notification alert in the panel to jump directly to that title's "
	        "detail page. "
	        "Ongoing series are checked every day. "
	        "Ended series - those with a complete year range such as 2007-2013 - are "
	        "checked once every 30 days since new seasons are rare but not impossible.",
	    },
	    {
	        "Sorting and ordering the library",
	        "Click the sort button in the top bar to change how titles are ordered in "
	        "your library. "
	        "A-Z arranges titles alphabetically by name. "
	        "Release orders them by release year. "
	        "Watch date orders them by when you last marked a title as watched. "
	        "Rank arranges them by your personal ranking score.",
	    },
	    {
	        "Ranking and comparing titles",
	        "Click the rank button in the top bar to start ranking all watched titles "
	        "that do not have a rank yet. "
	        "The app shows two titles side by side and asks which you prefer. "
	        "It uses binary search internally so you only need a small number of "
	        "comparisons "
	        "to find the correct position for each title in your ranked list. "
	        "Click the title you prefer and repeat until all titles are ranked. "
	        "You can exit ranking at any time with the X button - "
	        "titles already ranked in this session keep their position.",
	    },
	    {
	        "Adding a movie or TV show",
	        "Click the + button in the top bar to enter search mode. "
	        "Type a movie or TV show (series) name to search the OMDb database. "
	        "Click the plus button to add the title to your library. "
	        "If the title is already in your library, the same button removes it "
	        "instead.",
	    },
	    {
	        "Uploading a custom poster image",
	        "If a title has no poster artwork, hover over its card in the library to "
	        "reveal action buttons. "
	        "An upload button appears when the poster is missing. "
	        "Click it to select any image file from your computer to use as the poster "
	        "cover for that title. "
	        "The same upload button is also available on the title's detail page.",
	    },
	    {
	        "Removing or clearing a rank",
	        "Hover over a ranked title card in the library to reveal action buttons. "
	        "Click the remove ranking button to clear that title's rank score. "
	        "The title stays in your library and can be re-ranked at any time by "
	        "starting a new ranking session. "
	        "This button is also available on the title's detail page.",
	    },
	    {
	        "Streaming platforms - where to watch",
	        "Open a title's detail page and look at the Watch On section. "
	        "Click a streaming platform button to open a search for that title on the "
	        "corresponding service in your browser. "
	        "Click Try all to open every configured streaming platform at once. "
	        "You can add custom streaming services in Settings.",
	    },
	    {
	        "Adding a custom streaming platform or service",
	        "Open Settings and go to the Platforms tab. "
	        "Click Add, then enter the platform name, a search URL, and optionally "
	        "upload a logo image. "
	        "In the URL, write rewatch as a placeholder where the title should go - "
	        "the app replaces it with the actual title name when you click the platform "
	        "button. "
	        "Example: https://www.example.com/search?q=rewatch",
	    },
	    {
	        "Exporting and backing up your library",
	        "Go to Library > Export library in the menu bar. "
	        "Choose where to save the backup file. "
	        "The export is a .zip archive containing your full library data and all "
	        "poster images. "
	        "Use it as a backup or to copy and transfer your library to another "
	        "computer.",
	    },
	    {
	        "Importing and restoring a library backup",
	        "Go to Library > Import library in the menu bar and select a .zip backup "
	        "file. "
	        "This completely replaces your current library with the one in the backup "
	        "file. "
	        "This action cannot be undone, so use Export library first if you want to "
	        "save a copy of your current data.",
	    },
	    {
	        "Light and dark theme",
	        "Open Settings and go to the Appearance tab. "
	        "Click Light or Dark to switch between the two color themes. "
	        "The theme change applies immediately across the entire app without "
	        "restarting. "
	        "Each theme has its own independent accent color.",
	    },
	    {
	        "Changing the accent color",
	        "Open Settings and go to the Appearance tab. "
	        "Click Change color to open a color picker and choose a custom accent color. "
	        "The accent color is used for buttons, active borders, and highlights "
	        "throughout the app. "
	        "Light and dark themes each store their own accent color independently. "
	        "Click Reset to revert to the default accent color for the current theme.",
	    },
	    {
	        "Setting up your OMDb API key",
	        "ReWatch uses the OMDb API to search for movies and TV shows. "
	        "You need a free API key to enable the search feature. "
	        "Go to omdbapi.com to request a free key, then open Settings > API Key, "
	        "paste your key into the field, and click Add Key to activate it.",
	    },
	    {
	        "Daily title update limit",
	        "Each time ReWatch launches it checks your tracked TV shows for new seasons. "
	        "The free OMDb API allows up to 1 000 requests per day. "
	        "The daily title update limit (Settings > API Key) caps how many shows are "
	        "checked in a single day so you stay within that quota. "
	        "Shows that could not be checked because the limit was reached are "
	        "automatically prioritised on the next launch. "
	        "The default limit is 500.",
	    },
	    {
	        "Resetting and clearing all rankings",
	        "Open Settings and go to the Rankings tab. "
	        "Click Reset Movies to clear all movie rank scores, "
	        "or Reset TV Shows to clear all TV show rank scores. "
	        "Both actions show a confirmation dialog before proceeding. "
	        "Resetting rankings cannot be undone - titles remain in your library but "
	        "lose their ranking position.",
	    },
	};
}
