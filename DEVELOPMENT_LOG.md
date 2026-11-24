# Development log

## 2026-11-21 21:30 +03:00 — DPI scale, window geometry and OS helpers, on top of upstream's themes and fonts

### Changes made
- Started on top of origin/master a282281 (upstream's Fonts, Icons, Theme, FileDialog, Zoom and more).
  - The first version of this branch was written before those existed. It had its own `Theme` module
    (Breeze themes) and its own font loading.
  - Those were dropped in favour of upstream's. The rest was re-applied on top: the work was
    uncommitted, so it was stashed, the branch fast-forwarded, and the pieces re-applied by hand.
- `Window` additions:
  - The window is created scaled to its monitor (`GLFW_SCALE_TO_MONITOR`).
  - Content scale, with a `ContentScaleCb` when it changes.
  - `WaitEventsTimeout`.
  - The real framebuffer size at start.
  - `Geometry` get/set (the normal place, kept with pos/size callbacks; set is ignored when off every
    monitor).
  - `setDarkTitleBar`.
- `Gui` additions:
  - Scale: the monitor's × `setUserScale`.
  - `setFonts(ui, mono)` and `setFontSize`, through upstream's `loadFonts` and `setFontSize`.
  - `setTheme(key)`, through upstream's `Theme::find`/`setCurrent`/`applyCurrent`, plus a "system" key.
  - `refreshStyle` for `ThemeEditor`, and `waitEvents`.
- New `Platform` module: UTF-8 args, native file dialogs, `openUri`, `normalPath`, `prefersDarkTheme`,
  `setDarkTitleBar`, `systemUiFont`. It does not duplicate upstream's `configDir` or `defaultMonoFont`.
- Windows link libraries: comdlg32, shell32, advapi32, dwmapi.

### Features implemented
- **DPI:** the UI follows the monitor's scale, live when the window moves to another monitor, times a
  user scale (0.5–3).
- **Fonts:** by default the system's interface font (Segoe UI, or fontconfig's sans-serif) instead of
  ElenaWeb, with upstream's icon font merged in.
- **"system" theme:** upstream's Dark or Light, following the OS setting while the program runs. On
  Windows the title bar matches the theme.
- **Window geometry:** place, size and maximized state can be saved and restored.
- **`Gui::waitEvents`:** waits for events but draws a few frames per wake, so ImGui's queued input is
  handled.

### How it works
- `Gui::restyle` runs before `NewFrame` when something changed:
  - fonts first (`loadFonts`);
  - then the theme (`Theme::find`);
  - then the style: defaults → `Theme::applyCurrent` → `setFontSize` → `ScaleAllSizes(scale)` →
    `FontScaleDpi`.
- Dynamic fonts bake glyphs at the drawn size, so a scale change only restyles.
- Linux dialogs: zenity with `|` as the separator (dash can't quote a newline), or kdialog.

### Testing
- Through pdfview-app on Windows 11 at 4K, 150% scale:
  - The UI scales, with the system font and icon toolbar buttons.
  - Dark and Light themes switch live, including the title bar.
  - The Fonts window lists the installed fonts and switches the interface font live.
  - The native Open dialog works.
  - Geometry and maximized state are restored.
- Hd's own tests (search, vt100, zoom, theme, font) all pass, built inside pdfview with `-DHD_BUILD_TESTS=ON`.
- Not tested: Linux, macOS, the web build, moving between monitors with different scales.

### Outstanding issues
- `Gui` now loads the system font at start (upstream's default was the built-in ElenaWeb), so apps that
  never call `setFonts` look different. pdforg and logic need a check.
- Dark-mode detection is cached on Linux for 10 s.

### Next steps
- Build logic, pdforg and the web build against this branch.
- Run Hd's tests.
- Test the Linux paths (fc-match, zenity/kdialog, gsettings).

## 2026-11-22 19:24 +03:00 — Provide an optional bold UI face

- Gui loads a bold face with the other UI fonts. It uses the system choice for the default UI
  font and a matching Bold style for an explicitly chosen installed family. boldFont() returns
  a null pointer when a separate face is unavailable, allowing the host to supply its fallback.
- Built through pdfview on Windows with UCRT64. All six CTest targets passed. A Windows OpenGL
  integration check confirmed the default bold face, a live change to Arial Bold, and the
  unavailable-face fallback. Font discovery outside Windows was untested.

## 2026-11-23 23:37 +03:00 — File locks between processes and atomic file replacement

- **Features implemented:** `Hd/Files.h`, for files that several processes share, such as
  an app's settings.
  - `FileLock(path, timeout)`: an exclusive lock on a lock file, waiting up to the timeout
    (10 ms polls). Windows: `LockFileEx` on one byte; elsewhere `flock`, which belongs to
    the open file, so two locks conflict even within one process. The OS frees it when the
    process exits or crashes. Move-only; always taken in the browser.
  - `replaceFile(path, contents)`: writes a uniquely named temporary file in the same folder,
    flushes it (`FlushFileBuffers` / `fsync`, and the folder on POSIX), then moves it over
    the target (`MoveFileExW` with `MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH` /
    `rename`). On Windows it retries for about 200 ms while a reader holds the old file
    without delete sharing. On failure the old file is untouched and the temporary removed.
- **How it works:** Two primitives rather than one "safe save", so a caller can hold the
  lock across read, merge and replace (pdfview's settings do). The folder must exist.
- **Testing:** New `tests/files_test.cpp` (`hd_files_test`): replacing, overwriting and
  emptying a file with no temporaries left; a missing folder failing without creating it; on
  Windows, a reader blocking the move leaves the old contents readable and no temporary; a
  second lock in the same process timing out and succeeding after unlock; a move; a child
  process failing to take a held lock and taking it once released; and a child that exits
  holding the lock releasing it. Passed on Windows (UCRT64) through pdfview's build. Not
  run on Linux.
- **Recommended features:** The theme editor's saves can use `replaceFile`.

## 2026-11-24 00:54 +03:00 — normalPath reads MSYS drive paths on Windows

- **Changes made:** `Platform::normalPath("/c/Users/...")` returned `C:\c\Users\...`:
  `std::filesystem::absolute` reads `/c/...` as a folder `c` on the current drive. Git Bash
  usually converts such arguments before starting a Windows program, but launchers, scripts
  with `MSYS_NO_PATHCONV` and some tools pass them unchanged (seen in pdfview).
- **How it works:** On Windows, before normalising, a path that is `/` plus one letter, alone
  or followed by `/` or `\`, or the same after `/cygdrive`, becomes that drive (`/c/x` ->
  `C:/x`), but only when the drive's root exists; otherwise it may be a real folder and is
  left alone. Other systems are unchanged.
- **Testing:** New `tests/platform_test.cpp` (`hd_platform_test`): `/c/...`, `/C/...`,
  `/cygdrive/c/...` and `/c` on the system drive; an ordinary Windows path with `..`; `/cc/...`
  staying a folder; a drive letter that doesn't exist staying a folder. Passed on Windows
  through pdfview's build. Not run on Linux, where only an ordinary path is checked.

## 2026-11-24 09:55 +03:00 — hasFileDialog; one Linux dialog program; typed folders in FileList

- **Changes made:**
  - `Platform::openFilesDialog` on Linux ran zenity and, when its answer was empty, kdialog.
    Both exit with 1 on Cancel, so on a machine with both, cancelling showed a second dialog.
    It now picks one program first and runs only that.
  - With neither installed, the dialog returned nothing and the caller couldn't tell why.
  - `FileList::go("a/b/")` kept the trailing separator (`lexically_normal` keeps it), so the
    path bar showed the whole path as one part, and the parent was `a/b` again.
- **Features implemented:** `Platform::hasFileDialog()`: true on Windows; on Linux when zenity
  or kdialog is on the PATH; false in the browser. Callers use FileDialog.h's dialog
  otherwise (pdfview-app does).
- **How it works:** `onPath` looks for an executable in each PATH folder (`access(X_OK)`).
  `dialogProgram` tries kdialog first when `XDG_CURRENT_DESKTOP` contains KDE, zenity first
  elsewhere. `FileList::go` drops a trailing empty component unless the path is a root.
- **Testing:** `hd_platform_test` now runs fake `zenity` and `kdialog` shell scripts on a
  PATH of their own. It checks: neither present (no dialog, no answer); kdialog alone (two
  paths on lines, the filter passed as `PDF documents (*.pdf *.PDF)`); both outside KDE
  (zenity runs, `|`-separated paths, filter argument quoted correctly, kdialog not run);
  zenity cancelled (empty, kdialog not run); and KDE (kdialog runs). Passed on Linux (Arch
  under WSL). The FileList fix was checked by hand in pdfview-app on a VNC display: a typed
  `.../dir with spaces/` shows each folder in the path bar. Not built on Windows.
- **Outstanding issues:** `openUri` waits for `xdg-open`, which can block without a desktop
  environment. The real zenity and kdialog were not run.

## 2026-11-24 15:20 +03:00 — File browser: places, folder mode, several files, worker listing; openFolderDialog; openUri doesn't wait

- **Changes made:**
  - `FileList` and `FileDialog` turned names into text with `path::string()`, which is the ANSI code
    page on Windows (non-ASCII names broke). They use UTF-8 now (`toUtf8`/`fromUtf8`).
  - `Platform::openUri` on Linux waited for `xdg-open`; without a desktop environment that runs the
    program itself, so the app froze until it closed. It returns once the program started; a thread
    reaps it.
  - `FileList` is no longer copyable (a read may be in progress); `FileDialog` keeps its dialogs by
    pointer. The Windows drive combo in the dialog is gone: drives are places now.
- **Features implemented:**
  - `FileList.h` without UI: `toUtf8`, `fromUtf8`, `naturalLess`, `isHidden` (dot names; Windows'
    hidden and system attributes), `places()` (Home, the user's folders, then drives with labels on
    Windows or `/`), `parseUserDirs` (XDG user-dirs.dirs), `typedPath` (`~`, `C:`, relative names,
    MSYS `/c/...`, UNC), `completeFolder` (Tab).
  - `FileList`: reads on a worker thread (`glfwPostEmptyEvent` when done; synchronous in the browser),
    "Reading..." and error rows, `showHidden`, keys (type-ahead, arrows, Home/End, Enter, Backspace,
    Alt+Up), multiple selection (`multiple`, `selection`; Ctrl/Shift+click, Shift+arrows), an editable
    path bar (click its empty end or Ctrl+L; Tab completes; Enter goes; Escape stops).
  - `FileDialog`: places and recent folders on the left (`setRecentFolders`/`recentFolders`; a
    chosen file's folder becomes the newest), `openFolderDialog` (choose a folder: the typed one,
    else the selected one, else the one shown), `openFilesDialog` + `dialogResults`, "New folder"
    (save and folder modes), "Show hidden".
  - `Platform::openFolderDialog`: `IFileOpenDialog` with `FOS_PICKFOLDERS` on Windows, zenity
    `--file-selection --directory --filename=<start>/`, kdialog `--getexistingdirectory <start>`.
- **How it works:** a `FileList::Listing` (shared with a detached thread) lists everything; the UI
  thread takes it when done and filters it (`show`, hidden) itself, so the host's `show` callback
  never runs on the worker. A read for another folder is dropped when it ends. Windows links ole32
  and uuid now (known folders, COM dialog). On Windows a UNC path typed into the path bar is kept as
  typed (libstdc++ doesn't know UNC roots; `lexically_normal` merged the two separators).
- **Testing:** new `hd_filelist_test`: UTF-8 round trip, natural order, user-dirs parsing ($HOME,
  turned-off folders, order, CRLF), typed paths (relative, `..`, `~`; Windows: `C:`, `D:\a\..\b`, UNC,
  `/c/Windows`), completion (case, common part, one match gets a separator, files and hidden folders
  not offered unless asked, UTF-8 names), hidden attribute on Windows, places. `hd_platform_test`:
  fake zenity/kdialog for `openFolderDialog` (arguments, answer, cancel), and a fake `xdg-open` that
  sleeps to check `openUri` returns at once. All Hd tests pass on Linux and Windows (MSYS2 UCRT64;
  hd_font_test not run there). Checked by hand in pdforg on VNC: places with an XDG user dir, typed
  path with Tab and Enter, choose a folder, open three files with Shift+click, type-ahead, Enter,
  Backspace. Not tested: the Windows dialog and places on a real desktop, real zenity/kdialog, UNC
  shares, New folder by hand.
- **Outstanding issues:** the native dialogs block the frame that opens them (as before). A UNC
  share's parent folders can't be walked up in the path bar with libstdc++.
