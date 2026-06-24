# ReWatch

A desktop app to track your movies and TV shows. Built with C++ and Qt6.

![screenshot](screenshots/ReWatchLibrary.png)

---

## Features

- **Library** — Browse your movies and TV shows in a poster grid, sortable by title, release date, last viewed, or rank. Zoom the card size in or out to your preference — it's remembered across sessions.
- **Search & add** — Search the OMDb database by title and add results to your library in one click. Results show the title, year, and plot, and are limited to movies and TV shows.
- **Title detail view** — Click any title to open a full detail page: poster, metadata, scrollable plot, and a Watch on section with your configured streaming platform buttons.
- **Watched tracking** — Mark titles as watched or unwatched. Filter the library to show only titles left to watch.
- **Missing poster recovery** — If OMDb has no poster for a title, the library card shows an upload button so you can pick a local image to use instead.
- **Streaming platforms** — Add custom streaming services in Settings with a name, logo, and a search URL. Clicking a platform button on a title opens that platform's search pre-filled with the title. A "Try all" button opens all platforms at once. Up to 10 platforms supported.
- **Season updates** — On launch, the app checks OMDb for new seasons on your tracked TV shows (checked daily). Shows with a new season are automatically reset to unwatched.
- **Notifications** — New seasons show up as a badge on the bell icon; click it for a dropdown listing each show with its poster, with a sound played when one arrives.
- **Light and dark theme** — Switch between themes in Settings. Each theme has its own independently configurable accent color.
- **Export / Import** — Back up your full library (titles, posters, API key) as a zip file and restore it on any machine.
- **Tournament ranking** — Head-to-head binary search ranking for watched titles. The app picks two titles and asks which you prefer; each answer narrows the search until the title finds its exact place in the ranking. A library of 1000 titles takes at most 10 comparisons per title. Movies and TV shows are ranked separately. Rankings can be reset per type in Settings. Individual titles can be unranked directly from the library card.

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

---

## Setup

On first launch, open **Settings** from the menu bar and enter your OMDb API key. Without it, search and season update checks will not work.

---

## Data storage

The app stores your library at:

```
~/Library/Application Support/ReWatch/ReWatch.json
```

Poster images are saved alongside it in:

```
~/Library/Application Support/ReWatch/Posters/
```

Streaming platform logos are saved in:

```
~/Library/Application Support/ReWatch/PlatformImages/
```

---

## Tests

The project has a [Qt Test](https://doc.qt.io/qt-6/qttest-index.html) suite — 31 suites, 300+ cases — covering:

- **Core logic** — storage and persistence, library import validation, sorting / filtering / fuzzy search, OMDb URL building and error detection, and season-update eligibility.
- **UI behavior** — widgets, bars, dialogs, and views, exercised through signals, state transitions, and visibility (not pixels).
- **Utilities** — JSON file IO, Levenshtein distance, and SVG recoloring.

The app code is compiled once into a shared object library (`ReWatchLib`) that both the app and the test runner link against.

Build and run the suite:

```bash
./scripts/test.sh
```

It prompts for an OMDb API key — press Enter to skip the network-dependent integration tests (those run against the live OMDb API and are otherwise skipped). Or configure it by hand:

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

- **Code review** — catching bugs, leaks, and edge cases that were missed.
- **Implementing changes that have already been scoped** — once a change is decided and understood, AI may write the diff, which then gets reviewed and adjusted rather than typed by hand.
- **Sounding board** — discussing tradeoffs before deciding what to build.
- **Test suite** — the automated Qt Test suite under `tests/` and the manual test plan were written by AI, working from the existing source.

