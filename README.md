# MovieTracker

A desktop app to track your movies and TV shows. Built with C++ and Qt6.

![screenshot](screenshots/MovieTrackerLibrary.png)

---

## Features

- **Library** — Browse your movies and TV shows in a poster grid, sortable by title, release date, last viewed, or rank.
- **Search & add** — Search the OMDb database by title and add results to your library in one click.
- **Watched tracking** — Mark titles as watched or unwatched. Filter the library to show only titles left to watch.
- **Season updates** — On launch, the app checks OMDb for new seasons on your tracked TV shows (every 14 days). Shows with a new season are automatically reset to unwatched.
- **Export / Import** — Back up your full library (titles, posters, API key) as a zip file and restore it on any machine.
- **Tournament ranking** — *(coming soon)* Head-to-head tournament to rank every title in your library.

---

## Requirements

- Qt 6.x (Core, Gui, Widgets, Network, Concurrent, Svg)
- CMake 3.16+
- C++20 compiler (clang or gcc)
- An [OMDb API key](https://www.omdbapi.com/apikey.aspx) (free tier available)

---

## Build

For development, build and run directly:

```bash
cmake -B build
cmake --build build
cd build && ./MovieTracker
```

### Bundle (macOS .app)

To produce a standalone `.app` with Qt dependencies bundled in, use the provided script. `macdeployqt` must be in your PATH.

Before running the script, switch to SHIPPED asset paths: open `include/utils/AssetsPaths.hpp`, comment out the **DEV** block and uncomment the **SHIPPED** block. Switch back after bundling for development.

```bash
./scripts/bundle.sh "MovieTracker"
```

This builds the binary, creates the `.app` structure, generates the icon, copies assets, runs `macdeployqt`, and cleans up the build directory. The result is a `MovieTracker.app` ready to run.

---

## Setup

On first launch, go to **Omdb API key → Set API Key** in the menu bar and enter your OMDb key. Without it, search and season update checks will not work.

---

## Data storage

The app stores your library at:

```
~/.local/share/movieTracker/movieTracker.json
```

Poster images are saved alongside it in:

```
~/.local/share/movieTracker/Posters/
```

---

## On AI usage

This project is hand-designed: the architecture, the Qt widget structure, and the feature decisions come from the developer, not a model. AI is used as a tool throughout development, mainly for:

- **Code review** — catching bugs, leaks, and edge cases that were missed.
- **Implementing changes that have already been scoped** — once a change is decided and understood, AI may write the diff, which then gets reviewed and adjusted rather than typed by hand.
- **Sounding board** — discussing tradeoffs before deciding what to build.

