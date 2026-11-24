// fonts: the ones installed on the system, choosing one and loading them
// into imgui

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

struct ImFont;

namespace Hd {

// a font in a file, index is the font in a collection (.ttc). no file means
// the app's built-in font.
struct FontRef {
    std::filesystem::path file;
    int index = 0;

    bool builtin() const { return file.empty(); }
    bool operator==(const FontRef&) const = default;
};

// "index:file" for settings files, empty for the built-in font
std::string toString(const FontRef& f);
FontRef parseFontRef(std::string_view s);

struct FontFace {
    std::string family;
    std::string style;
    FontRef ref;
    // all glyphs are equally wide
    bool mono = false;

    // "family style"
    std::string name() const;
};

// the installed fonts that can be drawn (truetype and opentype outlines),
// sorted by name. asks fontconfig if the build has it, scans the usual font
// folders otherwise. takes a moment, keep the result.
std::vector<FontFace> systemFonts();

// the system's default monospace font, the built-in font if there is none
FontRef defaultMonoFont();

// the bytes of the font file. empty if it can't be read or holds no font
// that can be drawn, error says why.
std::vector<unsigned char> readFont(const FontRef& f, std::string& error);

// a combo box listing the fonts with a filter and a "monospace only" switch
class FontPicker {
public:
    explicit FontPicker(bool monoOnly = false)
        : monoOnly(monoOnly)
    {
    }

    // builtin: the name shown for the built-in choice. true when f changed.
    bool gui(const char* label, FontRef& f, const std::vector<FontFace>& fonts, const char* builtin);

private:
    std::string filter;
    bool monoOnly;
};

// replaces imgui's fonts, call it between frames. ui becomes the default
// font, mono the one of monoFont(). the built-in ui font is builtinTtf (not
// copied, must stay alive), the library's default font if empty. the
// built-in mono font is the system's default monospace font or imgui's own.
// fonts that can't be loaded are replaced by the built-in ones, errors says
// why. glyphs are made when they are first drawn, at any size. with icons
// the library's icon font is merged into the ui font, see Icons.h. with
// fallback the fallbackFonts() are merged into both, for the glyphs they
// lack.
void loadFonts(const FontRef& ui, const FontRef& mono, std::string_view builtinTtf, std::vector<std::string>& errors, bool icons = true, bool fallback = true);

// fonts for the scripts interface fonts rarely have: Chinese, Japanese and
// Korean. the fewest installed fonts that cover them, as fontconfig picks
// them for each language or else known files (Microsoft YaHei and Malgun
// Gothic on Windows, PingFang, Noto Sans CJK...); empty when there are none.
// they are read once and kept, tens of MB, the first call takes a moment.
std::vector<FontRef> fallbackFonts();

// height of the text in pixels from the next frame on, for all fonts
void setFontSize(float size);

// for code, addresses and memory dumps: all glyphs are equally wide. the
// default font until loadFonts.
ImFont* monoFont();

// draws with monoFont() while it lives
struct MonoScope {
    MonoScope();
    ~MonoScope();
    MonoScope(const MonoScope&) = delete;
    MonoScope& operator=(const MonoScope&) = delete;
};

}
