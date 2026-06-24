#pragma once

#include <QStandardPaths>
#include <QString>

namespace AppPaths
{

inline QString dataDir()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

inline constexpr const char *dataFile = "ReWatch.json";
inline constexpr const char *postersDir = "Posters";
inline constexpr const char *platformImagesDir = "PlatformImages";

} // namespace AppPaths
