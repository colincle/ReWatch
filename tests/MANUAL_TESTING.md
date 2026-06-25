# ReWatch — Manual Test Plan

The automated QTest suite covers logic, storage, parsing and signal wiring. It
**cannot** verify what only a human can judge: visual layout, live theming,
animations, real OMDb network calls, native file/color dialogs, the clipboard,
the browser hand-off, audio, and window behaviour. This checklist covers every
action a user can take, derived from the source.

The sections are ordered the way a tester would actually go through the app:
each step sets up what the next one needs (configure the key before searching,
add a platform before checking its button, rank titles before unranking them).
Work top to bottom. Run it before tagging a release.

**Prerequisites**
- A valid OMDb API key — you'll set it in section 2.
- An internet connection for search, adding titles, and season updates.

---

## 1. First launch & window

- [ ] **1.1** Launch the app — main window opens at the last-saved size (1200×800 first run); library shown on the Movies tab
- [ ] **1.2** Resize the window — card grid reflows to fit the new width after a brief pause; columns/spacing recompute, no overlap
- [ ] **1.3** Resize, close, relaunch — window reopens at the size it was closed at
- [ ] **1.4** Press Ctrl+W (Cmd+W on macOS) — app quits

## 2. Open Settings & set your API key

- [ ] **2.1** Library > Settings — Settings dialog opens (modal)
- [ ] **2.2** Click each tab (Appearance / API Key / Platforms / Rankings) — the matching section is shown
- [ ] **2.3** API Key field — pre-filled with the saved key (empty on first run)
- [ ] **2.4** API Key > empty / whitespace key — Add Key stays disabled; nothing saved
- [ ] **2.5** API Key > type your key, click Add Key — button shows "Adding…", disables, then re-enables; key is saved
- [ ] **2.6** API Key > Daily title update limit — spinner is present with default 900; change it to another value, close and reopen Settings — the saved value is shown

## 3. Add titles (search & add)

- [ ] **3.1** Click the **+** icon in the top bar — enters add mode (top bar replaced by the add bar); search field is auto-focused
- [ ] **3.2** Type a title and press Enter — spinner shows, then OMDb results appear (poster, title, year, plot)
- [ ] **3.3** Press Enter with empty / whitespace-only text — no search is run
- [ ] **3.4** Search a nonsense string — "No movies found" graphic is shown
- [ ] **3.5** Search before a key is set (or after temporarily clearing it) — error card appears with the API-key message
- [ ] **3.6** Click **Add** on a result — row shows a spinner, then the button switches to the "added" state; title is in the library
- [ ] **3.7** Click the **added** button — title is removed from the library; button reverts to Add
- [ ] **3.8** Add several titles (mix of movies and TV shows) so later sections have content to work with
- [ ] **3.9** Click the close button / press Escape — returns to the library
- [ ] **3.10** Add a movie whose OMDb "Released" date is in the future — the title appears in the library immediately; no notification is raised at this point

## 4. Browse the library

- [ ] **4.1** Click the **Movies** / **TV shows** tabs — library switches to that type; active tab highlighted
- [ ] **4.2** Click the already-active tab — nothing changes (no flicker / re-query)
- [ ] **4.3** Click the **sort** icon — menu opens with A–Z, Release, Watch date, Rank
- [ ] **4.4** Pick each sort option — library reorders accordingly (Rank also hides unranked titles and shows a "#n" prefix)
- [ ] **4.5** Click **All** / **To watch** — library filters; active button highlighted; clicking the active one does nothing
- [ ] **4.6** Click **zoom in** / **zoom out** — cards grow/shrink (clamped between min and max sizes); size persists across restart
- [ ] **4.7** Hold a zoom button — auto-repeats while held
- [ ] **4.8** Click the **search** icon — search field appears and is focused; search icon replaced by a close (✕)
- [ ] **4.9** Type a query and press Enter — library filters to fuzzy matches
- [ ] **4.10** Clear the search text — library returns to showing all titles
- [ ] **4.11** Press Escape / click the close button — search closes, field clears, full library restored

## 5. Title cards (library grid)

- [ ] **5.1** Hover a card — action buttons fade in (delete; watch toggle; unrank if ranked; upload-poster if artwork missing)
- [ ] **5.2** Move the cursor off the card — all action buttons hide again
- [ ] **5.3** Click the card body — opens the title detail view
- [ ] **5.4** Click **To watch** — title marked watched; button swaps to **Watched**
- [ ] **5.5** Click **Watched** — title marked unwatched; button swaps back
- [ ] **5.6** Click the **upload poster** button (only shown when artwork is missing) — file picker opens; choosing an image sets it as the poster and the button disappears
- [ ] **5.7** Click the **delete** (trash) button — title removed from the library immediately
- [ ] _(the **unrank** button is covered in section 9, once titles have been ranked)_

## 6. Title detail view — before any platforms are configured

- [ ] **6.1** Click a title to open it — poster, title, type (Movie / Series), metadata rows, plot, and "Watch on" section render
- [ ] **6.2** "Watch on" section with no platforms configured yet — "No custom streaming platforms…" hint is shown instead of buttons
- [ ] **6.3** Metadata shows Watched **Yes** for a watched title, **No** for an unwatched one
- [ ] **6.4** Toggle **To watch / Watched** — watched state and the "Last watched" row update live
- [ ] **6.5** Click **back** — returns to the library
- [ ] **6.6** Click **delete** — title removed and view returns to the library

## 7. Streaming platforms (Settings)

- [ ] **7.1** Settings > Platforms — the list shows each existing platform with logo + name + a delete button
- [ ] **7.2** Click **Add** (available while fewer than 10 platforms exist) — the add-platform dialog opens
- [ ] **7.3** Dialog: type a Name and Search URL — **Add** enables only when both are filled
- [ ] **7.4** Dialog: type a name that already exists — "already exists" error shows; Add stays disabled
- [ ] **7.5** Dialog: click **Copy "rewatch"** — clipboard contains `rewatch`; button shows "Copied!" for ~1.5 s
- [ ] **7.6** Dialog: click the logo **browse** button — file picker opens; chosen path appears in the (read-only) field
- [ ] **7.7** Dialog: click **Add** — platform is added (logo copied into storage); the Settings list refreshes
- [ ] **7.8** Dialog: click **Cancel** — dialog closes; nothing added
- [ ] **7.9** Add platforms up to 10 — the **Add** button is hidden once 10 exist
- [ ] **7.10** Delete a platform — it disappears from the list (and from any title's "Watch on", verified next). Leave at least one platform configured for section 8.

## 8. Title detail view — with platforms configured

- [ ] **8.1** Open a title — the "Watch on" section now shows a button per configured platform
- [ ] **8.2** Click a streaming-platform button — opens the browser to that service's search for this title
- [ ] **8.3** Click **Try all** — opens every configured platform at once
- [ ] **8.4** Delete a platform in Settings, reopen a title — that platform's button is gone from "Watch on"

## 9. Ranking

- [ ] **9.1** Click the **rank** icon with watched-but-unranked titles present — enters the ranking view
- [ ] **9.2** Ranking view — two title cards side by side, "Which do you prefer?", progress "Ranking Movies: x / y"
- [ ] **9.3** Click the left or right card — choice recorded; next comparison shown (binary search — few comparisons per title)
- [ ] **9.4** Finish all movies — phase switches to "Ranking TV Shows" if any are pending
- [ ] **9.5** Resize the window during ranking — cards rescale to fit
- [ ] **9.6** Click **exit** mid-session — ranking closes; titles ranked so far keep their positions
- [ ] **9.7** Finish everything — ranking view closes, returns to library
- [ ] **9.8** Click the **rank** icon when everything is ranked / nothing is watched — a disabled "All titles are ranked" menu appears; ranking does not start
- [ ] **9.9** Hover a now-ranked card and click **unrank** — rank is cleared; other ranks shift up
- [ ] **9.10** Settings > Rankings > Reset Movies / Reset TV Shows — confirmation dialog; on confirm, that type's ranks are cleared
- [ ] **9.11** Settings > Rankings > cancel the confirmation — ranks unchanged

## 10. Backup & restore

- [ ] **10.1** Library > Export library — save dialog (.zip); a backup archive is written; failure shows a warning
- [ ] **10.2** Library > Import library — open dialog (.zip); file is validated first, then "overwrite" confirmation appears; on OK the library is replaced
- [ ] **10.3** Import: cancel the confirmation — nothing changes
- [ ] **10.4** Import an invalid / tampered zip — "Invalid file" warning appears immediately (before any confirmation); library unchanged
- [ ] **10.5** Export then Import the same file — library round-trips identically (titles, posters, platforms, key, ranks)

## 11. Notifications & library updates

- [ ] **11.1** Launch with a key set and a TV series due for a season check — full-screen "Looking for new TV show seasons…" overlay with a spinner appears; menu bar disabled during it, re-enabled after
- [ ] **11.2** A tracked series gains a new season — a red dot appears on the bell and a notification sound plays
- [ ] **11.3** Click the **bell** icon — notifications panel pops up under the bell
- [ ] **11.4** Panel with notifications — alerts listed with poster, title, and a colored type label: **New Season** for a new series season, **New Episode** for a new episode within the same season, **Now available** for a movie that has released
- [ ] **11.5** Click a notification — navigates to that title's detail page; panel closes
- [ ] **11.6** Open the panel (with notifications) — red dot clears after opening
- [ ] **11.7** No notifications — panel shows "No notifications"; no red dot
- [ ] **11.8** Add a movie with a future release date; wait for (or manually set) that date to arrive and trigger an update — a **Now available** notification appears with the correct label (not "New Season")

## 12. Help

- [ ] **12.1** Help > How to use — Help dialog opens; all topics are listed
- [ ] **12.2** Type in the search box — list filters to matching topics (case-insensitive)
- [ ] **12.3** Clear the search box — all topics shown again

## 13. Appearance & theming regression sweep

- [ ] **13.1** Settings > Appearance > toggle Light / Dark — theme changes live across the whole app; no restart, no leftover colors
- [ ] **13.2** Settings > Appearance > Change color — color picker opens; chosen accent applies live and is stored per-theme
- [ ] **13.3** Settings > Appearance > Reset — accent reverts to the theme default; the Reset button hides when already default
- [ ] After changing theme or accent, click through every screen and confirm colors update with no leftovers:
  - [ ] **13.4** Top bar + tab buttons
  - [ ] **13.5** Library grid + title cards
  - [ ] **13.6** Add bar + search results
  - [ ] **13.7** Title detail (incl. streaming buttons)
  - [ ] **13.8** Ranking view
  - [ ] **13.9** Settings + Add-platform dialog
  - [ ] **13.10** Help dialog
  - [ ] **13.11** Notifications panel
