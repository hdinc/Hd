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
