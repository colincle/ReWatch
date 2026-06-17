#pragma once

// -----------------------------------------------------------------------------
// Asset paths — switch between the two blocks depending on the build target.
//
// DEV:      running the binary directly from build/ during development.
// SHIPPED:  running inside a .app bundle produced by scripts/bundle.sh.
// Uncomment the SHIPPED block and comment out the DEV block before
// running the bundle script.
// -----------------------------------------------------------------------------

// -- DEV ----------------------------------------------------------------------
#define ASSETS_ROOT "../assets"
// -----------------------------------------------------------------------------

// -- SHIPPED ------------------------------------------------------------------
// #define ASSETS_ROOT "/../Resources/assets"
// -----------------------------------------------------------------------------

// Icons
#define ADD_ICON            ASSETS_ROOT "/icons/add.svg"
#define ADDED_ICON          ASSETS_ROOT "/icons/added.svg"
#define SEARCH_ICON         ASSETS_ROOT "/icons/search.svg"
#define RANK_ICON           ASSETS_ROOT "/icons/rank.svg"
#define SORT_ICON           ASSETS_ROOT "/icons/sort.svg"
#define CROSS_ICON          ASSETS_ROOT "/icons/cross.svg"
#define DELETE_ICON         ASSETS_ROOT "/icons/delete.svg"
#define VIEWED_ICON         ASSETS_ROOT "/icons/viewed.svg"
#define NOT_VIEWED_ICON     ASSETS_ROOT "/icons/notViewed.svg"
#define IMAGE_UPLOAD_ICON   ASSETS_ROOT "/icons/imageUpload.svg"
#define NOTIFICATIONS_ICON  ASSETS_ROOT "/icons/notifications.svg"
#define ZOOM_IN_ICON        ASSETS_ROOT "/icons/zoomIn.svg"
#define ZOOM_OUT_ICON       ASSETS_ROOT "/icons/zoomOut.svg"

// Images
#define POSTER_PLACEHOLDER  ASSETS_ROOT "/images/placeholderPoster.png"
#define NO_MOVIES_FOUND     ASSETS_ROOT "/images/noMoviesFound.png"

// Sounds
#define NOTIFICATION_SOUND  ASSETS_ROOT "/sounds/notification.wav"
