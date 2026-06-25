// User-facing error strings shown by ErrorCard. Centralised so wording stays consistent.
#pragma once

#include <QString>

inline const QString API_KEY_ERROR_MESSAGE =
    "Invalid or missing API key — open Settings from the menu bar to set your key. "
    "Get one at <a href=\"https://www.omdbapi.com/apikey.aspx\" style=\"color: "
    "white;\">omdbapi.com/apikey.aspx</a>.";

inline const QString SAVE_ERROR_MESSAGE =
    "Failed to save your library — check disk space and permissions.";

inline const QString SEARCH_NETWORK_ERROR_MESSAGE =
    "Couldn't reach OMDb — check your internet connection.";

inline const QString UPDATE_NETWORK_ERROR_MESSAGE =
    "Couldn't check for updates — check your internet connection.";

inline const QString RATE_LIMIT_ERROR_MESSAGE =
    "OMDb request limit reached — your daily quota is used up. Try again tomorrow.";
