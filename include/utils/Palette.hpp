#pragma once

#include <QString>

namespace Palette
{
inline QString bgPrimary;
inline QString bgSecondary;
inline QString surface;
inline QString border;
inline QString textPrimary;
inline QString textSecondary;
inline QString accent;
inline QString accentLight;
inline QString highlight;
inline QString success;
inline QString error;

namespace detail
{
inline void setDark()
{
	bgPrimary = "#0D1117";
	bgSecondary = "#161B22";
	surface = "#21262D";
	border = "#30363D";
	textPrimary = "#E6EDF3";
	textSecondary = "#8B949E";
	accent = "#7C5CFF";
	accentLight = "#A78BFA";
	highlight = "#F5C518";
	success = "#22C55E";
	error = "#EF4444";
}

inline void setLight()
{
	bgPrimary = "#F4F5F7";
	bgSecondary = "#E8EAED";
	surface = "#D8DCE2";
	border = "#B8BEC8";
	textPrimary = "#1F2328";
	textSecondary = "#656D76";
	accent = "#7C5CFF";
	accentLight = "#A78BFA";
	highlight = "#F5C518";
	success = "#22C55E";
	error = "#EF4444";
}
} // namespace detail

inline void setTheme(const QString &theme)
{
	theme == "light" ? detail::setLight() : detail::setDark();
}
} // namespace Palette
