#pragma once

#ifdef SHIPPED
#define ASSETS_ROOT_ "../Resources/assets"
#else
#define ASSETS_ROOT_ "../assets"
#endif

namespace AssetsPaths
{

// Icons
inline constexpr const char *addIcon = ASSETS_ROOT_ "/icons/add.svg";
inline constexpr const char *addedIcon = ASSETS_ROOT_ "/icons/added.svg";
inline constexpr const char *searchIcon = ASSETS_ROOT_ "/icons/search.svg";
inline constexpr const char *rankIcon = ASSETS_ROOT_ "/icons/rank.svg";
inline constexpr const char *sortIcon = ASSETS_ROOT_ "/icons/sort.svg";
inline constexpr const char *crossIcon = ASSETS_ROOT_ "/icons/cross.svg";
inline constexpr const char *deleteIcon = ASSETS_ROOT_ "/icons/delete.svg";
inline constexpr const char *boxNotCheckedIcon = ASSETS_ROOT_ "/icons/boxNotChecked.svg";
inline constexpr const char *boxCheckedIcon = ASSETS_ROOT_ "/icons/boxChecked.svg";
inline constexpr const char *imageUploadIcon = ASSETS_ROOT_ "/icons/imageUpload.svg";
inline constexpr const char *notificationsIcon = ASSETS_ROOT_ "/icons/notifications.svg";
inline constexpr const char *zoomInIcon = ASSETS_ROOT_ "/icons/zoomIn.svg";
inline constexpr const char *zoomOutIcon = ASSETS_ROOT_ "/icons/zoomOut.svg";
inline constexpr const char *copyIcon = ASSETS_ROOT_ "/icons/copy.svg";

// Images
inline constexpr const char *posterPlaceholder =
    ASSETS_ROOT_ "/images/placeholderPoster.png";
inline constexpr const char *noMoviesFound = ASSETS_ROOT_ "/images/noMoviesFound.png";

// Sounds
inline constexpr const char *notificationSound = ASSETS_ROOT_ "/sounds/notification.wav";

} // namespace AssetsPaths

#undef ASSETS_ROOT_
