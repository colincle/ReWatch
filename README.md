# ReWatch

A desktop app to track your movies and TV shows. Built with C++ and Qt6.

![screenshot](screenshots/ReWatchLibrary.png)

---

## Features

- **Library.** Browse your movies and TV shows in a poster grid, sortable by title, release date, last viewed, or rank. Zoom the card size in or out to your preference, and it is remembered across sessions.
- **Search & add.** Search the OMDb database by title and add results to your library in one click. Results show the title and plot, and are limited to movies and TV shows. Long result lists load in pages via a *Load more results* button. You can also paste an IMDb ID, or a full IMDb URL, to jump straight to a specific title when a title search is too broad or finds nothing.
- **Title detail view.** Click any title to open a full detail page: poster, metadata, scrollable plot, and a Watch on section with your configured streaming platform buttons.
- **Watched tracking.** Mark titles as watched or unwatched. Filter the library to show only titles left to watch.
- **Missing poster recovery.** If OMDb has no poster for a title, the library card shows an upload button so you can pick a local image to use instead.
- **Streaming platforms.** Add custom streaming services in Settings with a name, logo, and a search URL. Clicking a platform button on a title opens that platform's search pre-filled with the title. A "Try all" button opens all platforms at once. Up to 10 platforms supported.
- **Library updates.** On launch, the app checks OMDb for new content. TV shows are checked for new seasons and episodes: ongoing series daily, ended series every 30 days. Movies added before their release date are tracked and checked once the date arrives. Shows with new content are automatically reset to unwatched. A configurable daily request limit keeps usage within the OMDb free-tier quota. Titles that couldn't be checked are prioritised on the next launch.
- **Notifications.** New seasons, new episodes, and movie releases show up as a badge on the bell icon. Click it for a dropdown listing each title with its poster, and a sound plays when one arrives.
- **Light and dark theme.** Switch between themes in Settings. Each theme has its own independently configurable accent color.
- **Export / Import.** Back up your full library (titles, posters, API key) as a zip file and restore it on any machine.
- **Tournament ranking.** Head-to-head binary search ranking for watched titles. The app picks two titles and asks which you prefer, and each answer narrows the search until the title finds its exact place in the ranking. A library of 1000 titles takes at most 10 comparisons per title. Movies and TV shows are ranked separately. Rankings can be reset per type in Settings. Individual titles can be unranked directly from the library card.

---

## Requirements

- Qt 6.x (Core, Gui, Widgets, Network, Concurrent, Svg, Multimedia)
- CMake 3.16+
- C++20 compiler (clang or gcc)
- An [OMDb API key](https://www.omdbapi.com/apikey.aspx) (free tier available)

---

## Build

For development, build and run directly:

```bash
cmake -B build
cmake --build build
cd build && ./ReWatch
```

### Bundle (macOS .app)

To produce a standalone `.app` with Qt dependencies bundled in, use the provided script. `macdeployqt` must be in your PATH.

```bash
./scripts/bundle.sh "ReWatch"
```

This builds the binary, creates the `.app` structure, generates the icon, copies assets, runs `macdeployqt`, and cleans up the build directory. The result is a `ReWatch.app` ready to run.

### Bundle (Linux AppImage)

To produce a portable `.AppImage` with Qt bundled in, use the Linux script. It needs `qmake` for Qt6 in your PATH. `linuxdeploy` and its Qt plugin are downloaded automatically on first run and cached.

```bash
./scripts/bundle-linux.sh "ReWatch"
```

This builds the binary, assembles an AppDir, generates a desktop entry and icon, embeds Qt with `linuxdeploy`, and produces `ReWatch.AppImage` in the project root.

---

## Setup

On first launch, open **Settings** from the menu bar and enter your OMDb API key. Without it, search and season update checks will not work.

---

## Data storage

The app keeps everything in a single per-user data directory:

- **macOS**: `~/Library/Application Support/ReWatch/`
- **Linux**: `~/.local/share/ReWatch/`

Inside it:

- `ReWatch.json`: your library (titles, watched state, ranks, and the API key)
- `Posters/`: poster images
- `PlatformImages/`: streaming platform logos

Export / Import bundles this whole directory into a zip, so a backup made on one platform restores cleanly on the other.

---

## Tests

The project has a [Qt Test](https://doc.qt.io/qt-6/qttest-index.html) suite of 31 suites and 300+ cases, covering:

- **Core logic.** Storage and persistence, library import validation, sorting, filtering, fuzzy search, OMDb URL building and error detection, and library-update eligibility.
- **UI behavior.** Widgets, bars, dialogs, and views, exercised through signals, state transitions, and visibility (not pixels).
- **Utilities.** JSON file IO, Levenshtein distance, and SVG recoloring.

The app code is compiled once into a shared object library (`ReWatchLib`) that both the app and the test runner link against.

Build and run the suite:

```bash
./scripts/test.sh
```

It prompts for an OMDb API key. Press Enter to skip the network-dependent integration tests (those run against the live OMDb API and are otherwise skipped). Or configure it by hand:

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
cd build && ./tests/ReWatchTests
```

Any failures are written to `build/test_results.log`.

For the GUI and visual behavior automated tests can't judge (layout, live theming, animations, native dialogs, the browser hand-off), see the manual checklist in [`tests/MANUAL_TESTING.md`](tests/MANUAL_TESTING.md).

---

## On AI usage

This project is hand-designed: the architecture, the Qt widget structure, and the feature decisions come from the developer, not a model. AI is used as a tool throughout development, mainly for:

- **Code review.** Catching bugs, leaks, and edge cases that were missed.
- **Implementing changes that have already been scoped.** Once a change is decided and understood, AI may write the diff, which then gets reviewed and adjusted rather than typed by hand.
- **Sounding board.** Discussing tradeoffs before deciding what to build.
- **Test suite.** The automated Qt Test suite under `tests/` and the manual test plan were written by AI, working from the existing source.

---

## License

Released under the [MIT License](LICENSE).
