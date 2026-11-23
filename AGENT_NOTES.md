# Hd — agent notes

## What this is
A small C++ graphics and UI base library:
- a GLFW window with an OpenGL 3.3 context;
- Dear ImGui setup and look: scale, fonts and icons, themes;
- ImGui widgets: file dialog, file list, terminal (VT100), canvas, zoomable windows;
- 2D/3D drawing helpers, MSDF text, embedded resources.

CMake. It also builds for the web with Emscripten. Used by pdfview, pdforg and logic, usually as a git
submodule.

## Map
- `include/Hd/`, `src/Hd/`: the library.
  - `Window`: a singleton GLFW window. Callbacks, `run` (also in the browser), content scale, geometry,
    dark title bar.
  - `Gui`: the ImGui frame, and the look. Scale is the monitor's times the user's; fonts and theme
    changes are applied between frames. `boldFont()` exposes the optional bold UI face. `waitEvents`.
  - `Theme`: Dark, Light, Black and custom JSON themes in `configDir()/themes`; the app colour table;
    `ThemeEditor`. Also `setConfigName`/`configDir`.
  - `Fonts`: installed fonts, `FontPicker`, `loadFonts`, `setFontSize`, `monoFont`.
  - `Icons`: Font Awesome solid glyphs, merged into the UI font; `iconButton`; toolbar sizes.
  - `Platform`: what the rest doesn't cover. UTF-8 args, native file dialogs (Win32, zenity/kdialog),
    `openUri`, dark-mode setting, dark title bar, the system's interface font.
  - `FileDialog`/`FileList`: an ImGui file dialog (works in the browser).
  - `Files`: `FileLock` (exclusive between processes: LockFileEx / flock, freed on exit) and
    `replaceFile` (temp file in the same folder, flushed, then moved over the old one).
  - `Zoom`, `Search`, `Vt100`/`TerminalView`, `Canvas2D`, `Web`.
- `deps/`: imgui (docking branch, 1.92), glad, glad_web, hdres (resource packer), stb, msdfgen.
- `res/`: resources packed into the library (shaders, ElenaWeb and fa-solid-900 fonts).
- `tests/`: tests with a small CHECK macro (files, font, platform, search, theme, vt100, zoom). `files_test` runs
  copies of itself to test locks between processes. Built when Hd is the top-level project or
  with `HD_BUILD_TESTS`.
- `web/pre.js`: the page's side of `Web.h`.

## Commands
- `cmake -B build -G Ninja && cmake --build build && ctest --test-dir build`
- Needs glfw3, glm, nlohmann_json (themes), optionally freetype and fontconfig.

## Decisions
- UI sizes = the monitor's content scale × the user's scale. ImGui 1.92's dynamic fonts
  (`FontScaleDpi`) rescale without reloading fonts.
- `Gui::restyle` order: ImGuiStyle defaults → `Theme::applyCurrent` → `setFontSize` → `ScaleAllSizes` →
  `FontScaleDpi`.
- `Gui` defaults: the "dark" theme, and the system's interface font (Segoe UI, fontconfig sans-serif)
  rather than the built-in ElenaWeb.
- `Gui::setTheme("system")` picks dark or light by the OS setting, re-checked every 2 s.
- Native file dialogs live in `Platform`; the ImGui one is for the web and for workspaces with a root.
- `Platform::normalPath` on Windows reads MSYS/Cygwin drive paths (`/c/...`, `/cygdrive/c/...`) as that drive when
  it exists, so paths from Git Bash scripts and launchers that skip conversion still open.

## Docs
- None yet.

## Current work
- Scale, window geometry, dark title bar, Platform and `waitEvents` are on `master` (41b19a8). They were
  tested on Windows through pdfview-app.
- Next: build logic, pdforg and the web build against it; test on Linux.

## Ruled out
- nativefiledialog-extended (GTK build dependency).
- A second theme system: upstream's `Theme` (JSON, editor) is the one.
