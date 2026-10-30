#pragma once

// icons of font awesome free, solid style (res/fonts/fa-solid-900.ttf of the
// library, sil ofl 1.1, icons cc by 4.0), merged into the interface font by
// loadFonts. each is the utf-8 of the icon's codepoint, see
// fontawesome.com/icons (free, solid) to add more; apps can add their own
// the same way.

namespace Hd {

namespace Icon {

inline constexpr const char* play = "\xef\x81\x8b"; // play
inline constexpr const char* pause = "\xef\x81\x8c"; // pause
inline constexpr const char* cancel = "\xef\x81\x97"; // circle-xmark
inline constexpr const char* undo = "\xef\x83\xa2"; // arrow-rotate-left
inline constexpr const char* redo = "\xef\x80\x9e"; // arrow-rotate-right
inline constexpr const char* upload = "\xef\x82\x93"; // upload
inline constexpr const char* download = "\xef\x80\x99"; // download
inline constexpr const char* folder = "\xef\x81\xbb"; // folder
inline constexpr const char* newFolder = "\xef\x99\x9e"; // folder-plus
inline constexpr const char* file = "\xef\x85\x9b"; // file
inline constexpr const char* rename = "\xef\x81\x84"; // pen-to-square
inline constexpr const char* trash = "\xef\x8b\xad"; // trash-can
inline constexpr const char* home = "\xef\x80\x95"; // house
inline constexpr const char* xmark = "\xef\x80\x8d"; // xmark
inline constexpr const char* plus = "\xef\x81\xa7"; // plus
inline constexpr const char* zoomIn = "\xef\x80\x8e"; // magnifying-glass-plus
inline constexpr const char* zoomOut = "\xef\x80\x90"; // magnifying-glass-minus
inline constexpr const char* fit = "\xef\x81\xbe"; // arrows-left-right
inline constexpr const char* eraser = "\xef\x84\xad"; // eraser
inline constexpr const char* flag = "\xef\x80\xa4"; // flag
inline constexpr const char* list = "\xef\x80\xba"; // list
inline constexpr const char* prev = "\xef\x81\x93"; // chevron-left
inline constexpr const char* next = "\xef\x81\x94"; // chevron-right
inline constexpr const char* go = "\xef\x81\xa1"; // arrow-right
inline constexpr const char* caretRight = "\xef\x83\x9a"; // caret-right
inline constexpr const char* caretDown = "\xef\x83\x97"; // caret-down
inline constexpr const char* unzoom = "\xef\x81\xa6"; // compress
inline constexpr const char* copy = "\xef\x83\x85"; // copy
inline constexpr const char* paste = "\xef\x83\xaa"; // paste
inline constexpr const char* terminal = "\xef\x84\xa0"; // terminal

}

// size of the toolbar buttons, chosen in their right click menu or with
// toolbarSizeMenu. apps keep it with their settings.
enum class ToolbarSize {
    small,
    medium,
    large
};
ToolbarSize getToolbarSize();
void setToolbarSize(ToolbarSize s);
// the side of a toolbar button in pixels, never smaller than a text button
float toolbarButtonSize();
// the space between toolbar buttons
float toolbarGap();
// the choices as menu items
void toolbarSizeMenu();

// a square toolbar button showing one icon, highlighted while active, greyed
// out unless enabled. the tooltip names it, since there is no text, and
// shows while it's greyed out too. the icon's visible shape is centered. a
// right click chooses the size. text after it on the same line is centered
// on it.
bool iconButton(const char* icon, const char* tip, bool active = false, bool enabled = true);
// the same at a given size, without the size menu
bool iconButtonAt(const char* icon, const char* tip, float size, bool active = false, bool enabled = true);
// an icon button as high as a text button (size 0) or of the given size,
// e.g. in a panel that doesn't follow the toolbar size
bool smallIconButton(const char* icon, const char* tip, float size = 0);

// moves the next control with a frame (checkbox, input) down to the middle
// of a toolbar line, text gets there by itself
void toolbarCenter();

}
