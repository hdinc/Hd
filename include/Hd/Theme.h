// themes: imgui's colors and the app's own named colors, with dark, light
// and black defaults. custom themes are json files in the user's config
// folder.

#pragma once

#include <imgui.h>

#include <array>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Hd {

// the user's settings folder of the app, e.g. ~/.config/<name>. apps that
// share settings (e.g. an editor and its viewer) set the same name.
void setConfigName(std::string name);
std::filesystem::path configDir();

// one of the app's colors besides imgui's own
struct ThemeColor {
    // in theme files
    const char* key;
    const char* name;
    // the heading it is listed under in the theme editor
    const char* group;
    // where it is used
    const char* description;
    ImVec4 dark;
    ImVec4 light;
    ImVec4 black;
};

struct Theme {
    std::string name;
    // the built-in theme missing colors come from: "dark", "light" or
    // "black"
    std::string base = "dark";
    // the app's colors, in the order of colorTable()
    std::vector<ImVec4> colors;
    // imgui's colors, by ImGuiCol
    std::array<ImVec4, ImGuiCol_COUNT> ui {};
    // the file of a custom theme, empty for the built-in ones
    std::filesystem::path file;

    bool builtin() const { return file.empty(); }
    ImVec4 operator[](size_t i) const { return i < colors.size() ? colors[i] : ImVec4 {}; }
    // a color of the table by its key, fallback if the app has none by that
    // name
    ImVec4 color(std::string_view key, ImVec4 fallback) const;
    // the name find() takes: a built-in key or the file name without .json
    std::string key() const;
    // writes the theme to file, creating the folder
    bool save(std::string& error) const;

    // the group of imgui's colors in the editor and the key in theme files
    static constexpr const char* interfaceGroup = "Interface";

    // the app's colors, once at the start. the current theme gets them from
    // its base.
    static void setColorTable(std::vector<ThemeColor> table);
    static const std::vector<ThemeColor>& colorTable();

    // the built-in themes by key and name: Dark, Light and Black
    static const std::vector<std::pair<std::string, std::string>>& builtinThemes();
    // a built-in theme by its key, Dark if there is none: the defaults of the
    // color table and imgui's colors (black: imgui's dark ones on black)
    static Theme makeBuiltin(const std::string& key);

    // the theme in use, Dark at the start
    static const Theme& current();
    static void setCurrent(Theme t);
    // imgui's colors of the current theme, with rounded frames, windows and
    // tabs, outlined inputs and buttons and lines between tree nodes
    static void applyCurrent();

    // where custom themes are kept, in configDir()
    static std::filesystem::path dir();
    // the custom themes' files in dir(), sorted
    static std::vector<std::filesystem::path> list();
    // a theme file. colors that are missing or invalid come from its base,
    // unknown keys are ignored. false with error set if it can't be read.
    static bool load(const std::filesystem::path& file, Theme& out, std::string& error);
    // a theme by its key (see key()). false with error set if it can't be
    // loaded, out is then Dark.
    static bool find(const std::string& name, Theme& out, std::string& error);

    // choosing a theme by its key: Dark, Light, Black and the custom themes.
    // true when the user picked another one.
    static bool combo(const char* label, std::string& key);
    static bool menuItems(std::string& key);
};

// "#rrggbbaa", and back ("#rrggbb" or "#rrggbbaa"). false if invalid.
std::string toHex(ImVec4 c);
bool parseHex(const std::string& s, ImVec4& out);

// edits the colors of the current theme in a window of its own. the
// built-in themes are read only, a duplicate is a custom theme saved in
// Theme::dir(). custom themes are named like their files.
class ThemeEditor {
public:
    bool open = false;

    // key: the theme chosen in the settings, changed when a theme is
    // duplicated, renamed or deleted. useCurrent: shows Theme::current()
    // after it changed. load: loads and shows the theme of key.
    void gui(std::string& key, const std::function<void()>& useCurrent, const std::function<void()>& load);

private:
    // asks for a name, then duplicates or renames
    enum class Naming { none, duplicate, rename };
    void namePopup(std::string& key, const std::function<void()>& load);
    void save();
    // false if a theme by that name can't be made
    bool checkName(const std::string& name);

    Naming asking = Naming::none;
    char nameBuf[64] = {};
    std::string error;
    ImGuiTextFilter filter;
};

}
