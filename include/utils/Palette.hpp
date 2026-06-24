// Global color variables for the current theme. Call setTheme() on launch and after
// theme changes; all widgets read from these globals when building their stylesheets.
#pragma once

#include <QColor>
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
inline QString success;
inline QString error;

inline constexpr const char *defaultAccent = "#7C5CFF";

inline QString lightenAccent(const QString &color)
{
	QColor c(color);
	return QColor::fromHslF(
	           c.hslHueF(),
	           c.hslSaturationF() * 0.9,
	           qMin(1.0, c.lightnessF() + 0.08)
	)
	    .name();
}

inline void setAccent(const QString &color)
{
	accent = color;
	accentLight = lightenAccent(color);
}

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
	success = "#22C55E";
	error = "#EF4444";
}

inline void setLight()
{
	bgPrimary = "#CAC7C2";
	bgSecondary = "#BFBBB6";
	surface = "#B0ACA5";
	border = "#938E86";
	textPrimary = "#1F2328";
	textSecondary = "#656D76";
	success = "#008430";
	error = "#EF4444";
}
} // namespace detail

inline void setTheme(const QString &theme)
{
	theme == "light" ? detail::setLight() : detail::setDark();
}
} // namespace Palette
