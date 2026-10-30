#include <Hd/Theme.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace Hd {

namespace {

std::string& configName()
{
    static std::string name = "Hd";
    return name;
}

std::vector<ThemeColor>& table()
{
    static std::vector<ThemeColor> t;
    return t;
}

Theme& currentTheme()
{
    static Theme t = Theme::makeBuiltin("dark");
    return t;
}

constexpr ImVec4 rgb(unsigned hex, float a = 1.0f)
{
    return { ((hex >> 16) & 255) / 255.0f, ((hex >> 8) & 255) / 255.0f, (hex & 255) / 255.0f, a };
}

// imgui's dark colors on black: windows, frames and title bars darker
void makeBlack(ImVec4* c)
{
    c[ImGuiCol_WindowBg] = rgb(0x000000);
    c[ImGuiCol_ChildBg] = rgb(0x000000, 0.0f);
    c[ImGuiCol_PopupBg] = rgb(0x0a0a0a, 0.97f);
    c[ImGuiCol_Border] = rgb(0x2e2e33);
    c[ImGuiCol_FrameBg] = rgb(0x141414);
    c[ImGuiCol_FrameBgHovered] = rgb(0x1f2a3a);
    c[ImGuiCol_FrameBgActive] = rgb(0x26364d);
    c[ImGuiCol_TitleBg] = rgb(0x000000);
    c[ImGuiCol_TitleBgActive] = rgb(0x0d1a2b);
    c[ImGuiCol_TitleBgCollapsed] = rgb(0x000000);
    c[ImGuiCol_MenuBarBg] = rgb(0x0a0a0a);
    c[ImGuiCol_ScrollbarBg] = rgb(0x000000);
    c[ImGuiCol_TableHeaderBg] = rgb(0x141414);
    c[ImGuiCol_TableBorderStrong] = rgb(0x2e2e33);
    c[ImGuiCol_TableBorderLight] = rgb(0x1e1e22);
    c[ImGuiCol_DockingEmptyBg] = rgb(0x000000);
    c[ImGuiCol_Separator] = rgb(0x2e2e33);
}

// the app's colors of a built-in theme
std::vector<ImVec4> builtinColors(const std::string& base)
{
    std::vector<ImVec4> r;
    for (auto& d : table())
        r.push_back(base == "light" ? d.light : base == "black" ? d.black : d.dark);
    return r;
}

// keys and names of the themes to choose from
std::vector<std::pair<std::string, std::string>> themeChoices()
{
    auto r = Theme::builtinThemes();
    for (auto& f : Theme::list())
        r.push_back({ f.stem().string(), f.stem().string() });
    return r;
}

}

void setConfigName(std::string name)
{
    configName() = std::move(name);
}

std::filesystem::path configDir()
{
    auto env = [](const char* name) -> std::filesystem::path {
        auto v = std::getenv(name);
        return v && *v ? std::filesystem::path(v) : std::filesystem::path();
    };
    auto& name = configName();
#if defined(_WIN32)
    if (auto d = env("APPDATA"); !d.empty()) return d / name;
#elif defined(__APPLE__)
    if (auto d = env("HOME"); !d.empty()) return d / "Library" / "Application Support" / name;
#else
    if (auto d = env("XDG_CONFIG_HOME"); !d.empty()) return d / name;
    if (auto d = env("HOME"); !d.empty()) return d / ".config" / name;
#endif
    return name + "-config";
}

ImVec4 Theme::color(std::string_view key, ImVec4 fallback) const
{
    auto& t = table();
    for (size_t i = 0; i < t.size() && i < colors.size(); i++)
        if (key == t[i].key) return colors[i];
    return fallback;
}

std::string Theme::key() const
{
    return builtin() ? base : file.stem().string();
}

void Theme::setColorTable(std::vector<ThemeColor> t)
{
    table() = std::move(t);
    auto& cur = currentTheme();
    auto defaults = builtinColors(cur.base);
    cur.colors.resize(defaults.size());
    for (size_t i = 0; i < defaults.size(); i++)
        cur.colors[i] = defaults[i];
}

const std::vector<ThemeColor>& Theme::colorTable()
{
    return table();
}

const std::vector<std::pair<std::string, std::string>>& Theme::builtinThemes()
{
    static const std::vector<std::pair<std::string, std::string>> r { { "dark", "Dark" }, { "light", "Light" }, { "black", "Black" } };
    return r;
}

Theme Theme::makeBuiltin(const std::string& key)
{
    bool light = key == "light", black = key == "black";
    Theme t;
    t.name = light ? "Light" : black ? "Black" : "Dark";
    t.base = light ? "light" : black ? "black" : "dark";
    t.colors = builtinColors(t.base);
    // without a context: imgui only writes into the given style
    ImGuiStyle s;
    if (light)
        ImGui::StyleColorsLight(&s);
    else
        ImGui::StyleColorsDark(&s);
    if (black) makeBlack(s.Colors);
    std::copy(std::begin(s.Colors), std::end(s.Colors), t.ui.begin());
    return t;
}

const Theme& Theme::current()
{
    return currentTheme();
}

void Theme::setCurrent(Theme t)
{
    currentTheme() = std::move(t);
}

void Theme::applyCurrent()
{
    auto& s = ImGui::GetStyle();
    auto& cur = currentTheme();
    std::copy(cur.ui.begin(), cur.ui.end(), std::begin(s.Colors));
    s.FrameRounding = 4.0f;
    s.GrabRounding = 4.0f;
    s.WindowRounding = 6.0f;
    s.PopupRounding = 4.0f;
    s.TabRounding = 4.0f;
    // outlined inputs and buttons, lines connecting tree nodes
    s.FrameBorderSize = 1.0f;
    s.TreeLinesFlags = ImGuiTreeNodeFlags_DrawLinesToNodes;
}

std::filesystem::path Theme::dir()
{
    return configDir() / "themes";
}

std::vector<std::filesystem::path> Theme::list()
{
    std::vector<std::filesystem::path> r;
    std::error_code ec;
    for (auto& e : std::filesystem::directory_iterator(dir(), ec))
        if (e.is_regular_file(ec) && e.path().extension() == ".json") r.push_back(e.path());
    std::sort(r.begin(), r.end());
    return r;
}

std::string toHex(ImVec4 c)
{
    auto b = [](float v) { return (int)(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };
    char s[16];
    std::snprintf(s, sizeof s, "#%02x%02x%02x%02x", b(c.x), b(c.y), b(c.z), b(c.w));
    return s;
}

bool parseHex(const std::string& s, ImVec4& out)
{
    if (s.size() != 7 && s.size() != 9) return false;
    if (s[0] != '#') return false;
    unsigned v[4] = { 0, 0, 0, 255 };
    for (size_t i = 0; i < (s.size() - 1) / 2; i++) {
        auto part = s.substr(1 + 2 * i, 2);
        if (!std::isxdigit((unsigned char)part[0]) || !std::isxdigit((unsigned char)part[1])) return false;
        v[i] = (unsigned)std::stoul(part, nullptr, 16);
    }
    out = { v[0] / 255.0f, v[1] / 255.0f, v[2] / 255.0f, v[3] / 255.0f };
    return true;
}

bool Theme::load(const std::filesystem::path& file, Theme& out, std::string& error)
{
    std::ifstream in(file);
    if (!in) {
        error = "can't open " + file.string();
        return false;
    }
    auto j = nlohmann::json::parse(in, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        error = file.string() + " isn't a theme file";
        return false;
    }

    auto base = j.value("base", std::string("dark"));
    out = makeBuiltin(base);
    out.name = j.value("name", file.stem().string());
    out.file = file;
    auto read = [](const nlohmann::json& obj, const char* key, ImVec4& c) {
        auto it = obj.find(key);
        if (it == obj.end() || !it->is_string()) return;
        ImVec4 v;
        if (parseHex(it->get<std::string>(), v)) c = v;
    };
    auto& t = table();
    if (auto it = j.find("colors"); it != j.end() && it->is_object())
        for (size_t i = 0; i < t.size(); i++)
            read(*it, t[i].key, out.colors[i]);
    if (auto it = j.find("interface"); it != j.end() && it->is_object())
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            read(*it, ImGui::GetStyleColorName(i), out.ui[i]);
    return true;
}

bool Theme::save(std::string& error) const
{
    nlohmann::ordered_json j;
    j["name"] = name;
    j["base"] = base;
    auto& c = j["colors"];
    // an object even when the app has no colors of its own
    c = nlohmann::ordered_json::object();
    auto& t = table();
    for (size_t i = 0; i < t.size() && i < colors.size(); i++)
        c[t[i].key] = toHex(colors[i]);
    auto& u = j["interface"];
    for (int i = 0; i < ImGuiCol_COUNT; i++)
        u[ImGui::GetStyleColorName(i)] = toHex(ui[i]);

    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    std::ofstream out(file);
    if (out) out << j.dump(2) << '\n';
    if (!out) {
        error = "can't write " + file.string();
        return false;
    }
    return true;
}

bool Theme::find(const std::string& name, Theme& out, std::string& error)
{
    for (auto& [key, label] : builtinThemes())
        if (name == key) {
            out = makeBuiltin(key);
            return true;
        }
    if (name.empty()) {
        out = makeBuiltin("dark");
        return true;
    }
    if (load(dir() / (name + ".json"), out, error)) return true;
    out = makeBuiltin("dark");
    return false;
}

bool Theme::combo(const char* label, std::string& key)
{
    auto choices = themeChoices();
    auto it = std::find_if(choices.begin(), choices.end(), [&](auto& c) { return c.first == key; });
    bool changed = false;
    if (ImGui::BeginCombo(label, it != choices.end() ? it->second.c_str() : key.c_str())) {
        for (size_t i = 0; i < choices.size(); i++) {
            // the custom themes after the built-in ones
            if (i == builtinThemes().size()) ImGui::Separator();
            if (ImGui::Selectable(choices[i].second.c_str(), choices[i].first == key) && choices[i].first != key) {
                key = choices[i].first;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool Theme::menuItems(std::string& key)
{
    bool changed = false;
    auto choices = themeChoices();
    for (size_t i = 0; i < choices.size(); i++) {
        if (i == builtinThemes().size()) ImGui::Separator();
        if (ImGui::MenuItem(choices[i].second.c_str(), nullptr, choices[i].first == key) && choices[i].first != key) {
            key = choices[i].first;
            changed = true;
        }
    }
    return changed;
}

namespace {

bool same(ImVec4 a, ImVec4 b)
{
    return toHex(a) == toHex(b);
}

ImVec4 errorColor()
{
    return Theme::current().color("error", { 1.0f, 0.3f, 0.3f, 1.0f });
}

}

bool ThemeEditor::checkName(const std::string& name)
{
    if (name.empty()) {
        error = "Give the theme a name";
        return false;
    }
    if (name.find_first_of("/\\:*?\"<>|") != std::string::npos || name[0] == '.') {
        error = "A name can't contain / \\ : * ? \" < > | or start with a dot";
        return false;
    }
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    for (auto& [key, label] : Theme::builtinThemes())
        if (lower == key) {
            error = label + " is a built-in theme";
            return false;
        }
    std::error_code ec;
    if (std::filesystem::exists(Theme::dir() / (name + ".json"), ec)) {
        error = "There is a theme named '" + name + "'";
        return false;
    }
    return true;
}

void ThemeEditor::save()
{
    std::string e;
    if (!Theme::current().save(e)) error = e;
}

void ThemeEditor::namePopup(std::string& key, const std::function<void()>& load)
{
    using namespace ImGui;

    if (asking != Naming::none && !IsPopupOpen("Theme name")) OpenPopup("Theme name");
    if (!BeginPopupModal("Theme name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

    TextUnformatted(asking == Naming::duplicate ? "Name of the copy" : "New name");
    if (IsWindowAppearing()) SetKeyboardFocusHere();
    bool enter = InputText("##name", nameBuf, sizeof nameBuf, ImGuiInputTextFlags_EnterReturnsTrue);
    if (!error.empty()) TextColored(errorColor(), "%s", error.c_str());

    bool ok = Button("OK") || enter;
    SameLine();
    if (Button("Cancel") || IsKeyPressed(ImGuiKey_Escape)) {
        asking = Naming::none;
        error.clear();
        CloseCurrentPopup();
    } else if (ok && checkName(nameBuf)) {
        auto t = Theme::current();
        auto old = t.file;
        t.name = nameBuf;
        t.file = Theme::dir() / (t.name + ".json");
        std::string e;
        if (t.save(e)) {
            // renamed: the old file goes
            std::error_code ec;
            if (asking == Naming::rename && !old.empty()) std::filesystem::remove(old, ec);
            key = t.key();
            load();
            error.clear();
            asking = Naming::none;
            CloseCurrentPopup();
        } else {
            error = e;
        }
    }
    EndPopup();
}

void ThemeEditor::gui(std::string& key, const std::function<void()>& useCurrent, const std::function<void()>& load)
{
    using namespace ImGui;
    if (!open) return;

    SetNextWindowSize({ GetFontSize() * 26, GetFontSize() * 32 }, ImGuiCond_FirstUseEver);
    if (!Begin("Theme editor", &open, ImGuiWindowFlags_NoDocking)) {
        End();
        return;
    }

    if (Theme::combo("Theme", key)) load();
    auto& cur = Theme::current();
    bool builtin = cur.builtin();

    if (Button("Duplicate...")) {
        asking = Naming::duplicate;
        auto name = cur.name + " copy";
        std::snprintf(nameBuf, sizeof nameBuf, "%s", name.c_str());
        error.clear();
    }
    SetItemTooltip("A custom theme starting with these colors");
    if (!builtin) {
        SameLine();
        if (Button("Rename...")) {
            asking = Naming::rename;
            std::snprintf(nameBuf, sizeof nameBuf, "%s", cur.name.c_str());
            error.clear();
        }
        SameLine();
        if (Button("Delete...")) OpenPopup("Delete theme");
    }
    namePopup(key, load);

    if (BeginPopupModal("Delete theme", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Text("Delete the theme '%s'?", cur.name.c_str());
        if (Button("Delete")) {
            std::error_code ec;
            std::filesystem::remove(cur.file, ec);
            // its base is the closest
            key = cur.base;
            load();
            CloseCurrentPopup();
        }
        SameLine();
        if (Button("Cancel") || IsKeyPressed(ImGuiKey_Escape)) CloseCurrentPopup();
        EndPopup();
    }

    if (builtin) TextDisabled("Built-in themes can't be changed, duplicate this one to edit it");
    if (!error.empty() && asking == Naming::none) TextColored(errorColor(), "%s", error.c_str());
    filter.Draw("Filter", GetFontSize() * 12);
    Separator();

    auto base = Theme::makeBuiltin(cur.base);
    auto& defs = Theme::colorTable();

    // the groups of the app's colors in the order of the table, imgui's last
    std::vector<const char*> groups;
    for (auto& d : defs)
        if (std::none_of(groups.begin(), groups.end(), [&](const char* g) { return std::strcmp(g, d.group) == 0; })) groups.push_back(d.group);
    groups.push_back(Theme::interfaceGroup);

    // one row: the color, its name and a reset to the base theme. true when
    // it changed.
    auto row = [&](const char* name, const char* description, ImVec4& c, ImVec4 def) {
        bool changed = false;
        PushID(name);
        BeginDisabled(builtin);
        if (ColorEdit4("##color", &c.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)) changed = true;
        bool done = IsItemDeactivatedAfterEdit();
        EndDisabled();
        SameLine();
        TextUnformatted(name);
        if (description && *description) SetItemTooltip("%s", description);
        if (!builtin && !same(c, def)) {
            SameLine();
            if (SmallButton("Reset")) {
                c = def;
                changed = done = true;
            }
            SetItemTooltip("Back to %s's color", base.name.c_str());
        }
        PopID();
        return std::pair { changed, done };
    };

    // the folder's line stays under the list
    BeginChild("colors", { 0, -GetFrameHeightWithSpacing() });
    Theme edited = cur;
    bool changed = false, done = false;
    for (auto group : groups) {
        bool uiGroup = std::strcmp(group, Theme::interfaceGroup) == 0;
        // groups with something passing the filter
        bool any = false;
        if (uiGroup) {
            for (int i = 0; i < ImGuiCol_COUNT && !any; i++)
                any = filter.PassFilter(GetStyleColorName(i));
        } else {
            for (auto& d : defs)
                any = any || (std::strcmp(d.group, group) == 0 && (filter.PassFilter(d.name) || filter.PassFilter(d.description)));
        }
        if (!any) continue;
        // filtering shows what matches
        if (filter.IsActive()) SetNextItemOpen(true);
        if (!CollapsingHeader(group, uiGroup ? 0 : ImGuiTreeNodeFlags_DefaultOpen)) continue;

        if (uiGroup) {
            for (int i = 0; i < ImGuiCol_COUNT; i++) {
                if (!filter.PassFilter(GetStyleColorName(i))) continue;
                auto [c, d] = row(GetStyleColorName(i), nullptr, edited.ui[i], base.ui[i]);
                changed |= c;
                done |= d;
            }
        } else {
            for (size_t i = 0; i < defs.size() && i < edited.colors.size(); i++) {
                auto& d = defs[i];
                if (std::strcmp(d.group, group) != 0 || !(filter.PassFilter(d.name) || filter.PassFilter(d.description))) continue;
                auto [c, dn] = row(d.name, d.description, edited.colors[i], base.colors[i]);
                changed |= c;
                done |= dn;
            }
        }
    }
    EndChild();
    auto folder = Theme::dir().string();
    TextDisabled("Custom themes: %s", folder.c_str());

    if (changed && !builtin) {
        Theme::setCurrent(std::move(edited));
        useCurrent();
    }
    // saved once a drag or text edit ends, not every frame
    if (done && !builtin) save();

    End();
}

}
