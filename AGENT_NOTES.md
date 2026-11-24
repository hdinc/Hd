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
  - `Fonts`: installed fonts, `FontPicker`, `loadFonts`, `setFontSize`, `monoFont`, `fallbackFonts` (CJK).
  - `Icons`: Font Awesome solid glyphs, merged into the UI font; `iconButton`; toolbar sizes.
  - `Platform`: what the rest doesn't cover. UTF-8 args, native file and folder dialogs (Win32,
    zenity/kdialog), `openUri` (doesn't wait), dark-mode setting, dark title bar, the system's interface font.
  - `FileDialog`/`FileList`: an ImGui file dialog (works in the browser): places and recent folders,
    open/save/folder/several files, worker-thread listing, keys, typed paths with Tab completion, hidden
    files, New folder. `FileList.h` also has the parts without UI (places, typedPath, completeFolder,
    UTF-8 path helpers), tested in `hd_filelist_test`.
  - `Files`: `FileLock` (exclusive between processes: LockFileEx / flock, freed on exit) and
    `replaceFile` (temp file in the same folder, flushed, then moved over the old one).
  - `Zoom`, `Search`, `Vt100`/`TerminalView`, `Canvas2D`, `Web`.
- `deps/`: imgui (docking branch, 1.92), glad, glad_web, hdres (resource packer), stb, msdfgen.
- `res/`: resources packed into the library (shaders; fonts: Noto Sans (OFL) built-in, fa-solid-900 icons,
  with their licence files). Added or removed files rerun cmake (`CONFIGURE_DEPENDS`).
- `tests/`: tests with a small CHECK macro (filelist, files, font, fonts, platform, search, theme, vt100, zoom). `files_test` runs
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
  rather than the built-in Noto Sans (the fallback, the web build's font and canvas text's default).
- `Gui::setTheme("system")` picks dark or light by the OS setting, re-checked every 2 s.
- Native file dialogs live in `Platform`; the ImGui one is for the web, for workspaces with a root, and as the
  fallback when `Platform::hasFileDialog()` is false (Linux without zenity or kdialog).
- Linux dialogs run one program, chosen before running it (kdialog first on KDE, zenity first elsewhere): both exit
  with 1 on Cancel, so trying the other after an empty answer showed a second dialog.
- `Platform::normalPath` on Windows reads MSYS/Cygwin drive paths (`/c/...`, `/cygdrive/c/...`) as that drive when
  it exists, so paths from Git Bash scripts and launchers that skip conversion still open.

## Docs
- None yet.

- CJK glyphs come from fallback fonts merged after the icons into the UI and mono fonts (`fallbackFonts`:
  per zh-cn/ja/ko, fontconfig's pick or known files, only if no chosen font has the script yet; read once
  per process, shared by both fonts). Windows: YaHei + Malgun Gothic (~33 MB in memory).
- Paths as text are UTF-8 everywhere (`toUtf8`/`fromUtf8`); never `path::string()` (ANSI on Windows).
- `FileList` reads on a worker thread, but filters (`show`, hidden) on the UI thread.

## Current work
- The file browser (2026-11-24, pdforg's W2) was checked on a Windows desktop in pdforg's W3: places,
  drives with labels, typed Unicode paths. CJK fallback fonts in a7ea8f8. Built-in font ElenaWeb (no
  licence) replaced by Noto Sans (uncommitted, for pdforg's W4 package).
- Next: build logic and the web build against it (the web build: FileList reads synchronously there).

## Ruled out
- nativefiledialog-extended (GTK build dependency).
- A second theme system: upstream's `Theme` (JSON, editor) is the one.
- ElenaWeb as the built-in font: no licence for shipping it. ImGui's ProggyVector instead: Latin-1 only,
  monospace look, and stored compressed (canvas text needs a TTF).
