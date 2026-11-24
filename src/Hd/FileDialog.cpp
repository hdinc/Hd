#include <Hd/FileDialog.h>
#include <Hd/FileList.h>
#include <Hd/Icons.h>
#include <Hd/Theme.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <memory>
#include <vector>

namespace Hd {

namespace fs = std::filesystem;

namespace {

struct file_filter {
    std::string label;
    // lower case with the dot, all files if empty
    std::vector<std::string> extensions;
};

enum class Mode { open, save, folder, files };

struct dialog {
    std::string title;
    Mode mode = Mode::open;
    std::vector<file_filter> filters;
    int filter = 0;
    FileList list;
    std::vector<Place> places; // read when it opens: drives' labels take a moment
    // the file name typed or chosen in the list
    std::string name;
    // name was changed here, the text field takes it even while it's edited
    bool name_set = false;
    std::string error;
    // OpenPopup at the next draw, in the caller's id stack
    bool pending = false;
    // drawn this frame
    bool visible = false;
    // asking before overwriting it
    fs::path replacing;
    // asking for a new folder's name
    bool naming = false;
    std::string new_folder;
    std::string new_folder_error;
};

std::map<std::string, std::unique_ptr<dialog>> dialogs;
fs::path dialog_root;
// the Home button's tooltip while there is a root
std::string root_name;
std::function<void(const fs::path&)> upload;
std::vector<fs::path> recent;
constexpr size_t max_recent = 8;

std::string lower(std::string s)
{
    for (auto& c : s)
        c = (char)std::tolower((unsigned char)c);
    return s;
}

// "Description{.a,.b},.*" into its filters
std::vector<file_filter> parse_filters(const std::string& s)
{
    std::vector<file_filter> r;
    size_t k = 0;
    while (k < s.size()) {
        auto brace = s.find('{', k);
        auto comma = s.find(',', k);
        file_filter f;
        if (brace != std::string::npos && (comma == std::string::npos || brace < comma)) {
            auto close = s.find('}', brace);
            if (close == std::string::npos) close = s.size();
            f.label = s.substr(k, brace - k);
            std::string list = s.substr(brace + 1, close - brace - 1);
            for (size_t i = 0; i <= list.size();) {
                auto j = std::min(list.find(',', i), list.size());
                if (j > i) f.extensions.push_back(lower(list.substr(i, j - i)));
                i = j + 1;
            }
            k = close + 1;
            if (k < s.size() && s[k] == ',') k++;
        } else {
            auto end = comma == std::string::npos ? s.size() : comma;
            auto e = s.substr(k, end - k);
            if (e == ".*") f.label = "All files";
            else {
                f.label = e;
                f.extensions.push_back(lower(e));
            }
            k = end + 1;
        }
        // the extensions in the label
        if (!f.extensions.empty() && f.label != f.extensions[0]) {
            f.label += " (";
            for (size_t i = 0; i < f.extensions.size(); i++)
                f.label += (i ? " " : "") + f.extensions[i];
            f.label += ")";
        }
        r.push_back(std::move(f));
    }
    if (r.empty()) r.push_back({"All files", {}});
    return r;
}

bool matches(const file_filter& f, const fs::path& p)
{
    if (f.extensions.empty()) return true;
    auto name = lower(toUtf8(p.filename()));
    return std::any_of(f.extensions.begin(), f.extensions.end(),
        [&](auto& e) { return name.size() > e.size() && name.compare(name.size() - e.size(), e.size(), e) == 0; });
}

bool inside(const fs::path& p, const fs::path& root)
{
    if (root.empty()) return true;
    auto rel = p.lexically_normal().lexically_relative(root.lexically_normal());
    return !rel.empty() && *rel.begin() != "..";
}

fs::path home()
{
    if (!dialog_root.empty()) return dialog_root;
    for (auto& p : places())
        if (p.icon == Icon::home) return p.path;
    std::error_code ec;
    return fs::current_path(ec);
}

// p relative to base if it's inside it
std::string relative_if_inside(const fs::path& p, const fs::path& base)
{
    if (base.empty()) return toUtf8(p.generic_u8string());
    std::error_code ec;
    auto rel = fs::relative(p, base, ec);
    if (ec || rel.empty() || *rel.begin() == "..") return toUtf8(p.generic_u8string());
    return toUtf8(rel.generic_u8string());
}

// sets up the dialog of key, shown at the next display(key)
void start(const std::string& key, const char* title, const char* filters, fs::path dir, const std::string& name, Mode mode)
{
    auto& slot = dialogs[key];
    slot = std::make_unique<dialog>();
    auto& d = *slot;
    d.title = title;
    d.mode = mode;
    d.filters = parse_filters(filters ? filters : ".*");
    d.list.root = dialog_root;
    d.list.multiple = mode == Mode::files;
    if (dialog_root.empty()) d.places = places();
    std::error_code ec;
    if (dir.empty() || !fs::is_directory(dir, ec) || !inside(dir, dialog_root)) dir = home();
    d.list.go(dir, name.empty() ? fs::path{} : dir / fromUtf8(name));
    if (mode == Mode::folder)
        d.list.show = [](const fs::path&) { return false; };
    else
        d.list.show = [&d](const fs::path& p) { return matches(d.filters[d.filter], p); };
    d.name = name;
    d.pending = true;
}

void remember(const fs::path& folder)
{
    if (folder.empty()) return;
    std::erase(recent, folder);
    recent.insert(recent.begin(), folder);
    if (recent.size() > max_recent) recent.resize(max_recent);
}

// the typed name as a path, relative to the folder shown
fs::path typed(const dialog& d)
{
    return typedPath(d.name, d.list.dir, home());
}

// what's typed or chosen: true with files if it's the result, else goes
// into a folder, asks before overwriting or sets an error
bool accept(dialog& d, std::vector<fs::path>& files)
{
    d.error.clear();
    std::error_code ec;
    if (d.mode == Mode::folder) {
        // the folder typed, else the one selected, else the one shown
        fs::path p = !d.name.empty() ? typed(d) : !d.list.selected.empty() ? d.list.selected : d.list.dir;
        if (!inside(p, dialog_root)) {
            d.error = "That's outside of " + toUtf8(dialog_root);
            return false;
        }
        if (!fs::is_directory(p, ec)) {
            d.error = "There's no folder '" + d.name + "'";
            return false;
        }
        files = {p};
        return true;
    }
    if (d.mode == Mode::files) {
        // the files selected in the list, when the name is one of them
        std::vector<fs::path> chosen;
        for (auto& p : d.list.selection)
            if (fs::is_regular_file(p, ec)) chosen.push_back(p);
        if (chosen.size() > 1 || (chosen.size() == 1 && toUtf8(chosen[0].filename()) == d.name)) {
            files = chosen;
            return true;
        }
    }
    if (d.name.empty()) {
        d.error = d.mode == Mode::save ? "Type a name for the file" : "Choose a file";
        return false;
    }
    auto p = typed(d);
    if (!inside(p, dialog_root)) {
        d.error = "That's outside of " + toUtf8(dialog_root);
        return false;
    }
    if (fs::is_directory(p, ec)) {
        d.list.go(p);
        d.name.clear();
        d.name_set = true;
        return false;
    }
    if (d.mode != Mode::save) {
        if (!fs::is_regular_file(p, ec)) {
            d.error = "'" + d.name + "' isn't there";
            return false;
        }
        files = {p};
        return true;
    }
    auto& f = d.filters[d.filter];
    if (!p.has_extension() && !f.extensions.empty()) p += fromUtf8(f.extensions[0]);
    if (!fs::is_directory(p.parent_path(), ec)) {
        d.error = "There's no folder " + toUtf8(p.parent_path());
        return false;
    }
    if (fs::exists(p, ec)) {
        d.replacing = p;
        return false;
    }
    files = {p};
    return true;
}

// the left side: places and recent folders
void placesPanel(dialog& d)
{
    using namespace ImGui;
    auto item = [&](const char* icon, const std::string& name, const fs::path& p, const std::string& tip) {
        PushID(toUtf8(p).c_str());
        bool here = d.list.dir == p;
        auto at = GetCursorScreenPos();
        if (Selectable("##place", here)) d.list.go(p);
        SetItemTooltip("%s", tip.c_str());
        SameLine();
        SetCursorScreenPos({at.x + GetStyle().FramePadding.x, at.y});
        TextUnformatted(icon);
        SameLine(0, GetFontSize() * 0.5f);
        TextUnformatted(name.c_str());
        PopID();
    };
    bool drives = false;
    for (auto& p : d.places) {
        if (p.drive && !drives) {
            drives = true;
            Spacing();
        }
        item(p.icon, p.name, p.path, toUtf8(fs::path(p.path).make_preferred()));
    }
    std::error_code ec;
    bool header = false;
    for (auto& r : recent) {
        if (!fs::is_directory(r, ec)) continue;
        if (!header) {
            header = true;
            Spacing();
            TextDisabled("Recent");
        }
        auto name = toUtf8(r.filename());
        item(Icon::folder, name.empty() ? toUtf8(r) : name, r, toUtf8(fs::path(r).make_preferred()));
    }
}

// draws the dialog of key, true once files (or a folder) were chosen
bool display(const std::string& key, std::vector<fs::path>& files)
{
    using namespace ImGui;

    auto it = dialogs.find(key);
    if (it == dialogs.end()) return false;
    auto& d = *it->second;
    auto popup = d.title + "###" + key;
    if (d.pending) {
        OpenPopup(popup.c_str());
        d.pending = false;
    }
    auto& vp = *GetMainViewport();
    SetNextWindowSize({vp.Size.x * 0.6f, vp.Size.y * 0.6f}, ImGuiCond_Appearing);
    bool open = true;
    d.visible = BeginPopupModal(popup.c_str(), &open, ImGuiWindowFlags_NoCollapse);
    if (!d.visible) {
        dialogs.erase(it);
        return false;
    }

    bool chosen = false;
    auto old_selection = d.list.selected;
    auto& style = GetStyle();
    float footer = GetFrameHeightWithSpacing() * 2.0f + (d.error.empty() ? 0.0f : GetTextLineHeightWithSpacing());

    // places on the left, unless the dialog keeps to a root
    if (!d.places.empty()) {
        BeginChild("places", {GetFontSize() * 11.0f, -footer}, ImGuiChildFlags_ResizeX);
        placesPanel(d);
        EndChild();
        SameLine();
    }
    BeginGroup();
    if (!dialog_root.empty()) {
        if (Button(Icon::home)) d.list.go(home());
        SetItemTooltip("%s", root_name.c_str());
        SameLine();
    }
    if (upload) {
        if (Button(Icon::upload)) upload(d.list.dir);
        SetItemTooltip("Upload files from your computer into this folder, replacing ones with the same names");
        SameLine();
    }
    d.list.pathBar();

    // the list over the options, the name and the buttons
    std::vector<fs::path> activated(1);
    if (d.list.table(activated[0], -footer)) {
        d.name = toUtf8(activated[0].filename());
        chosen = accept(d, files);
    }
    EndGroup();
    // a chosen file (or folder, choosing one) goes into the name
    std::error_code ec;
    if (d.list.selected != old_selection && !d.list.selected.empty()) {
        bool folder = fs::is_directory(d.list.selected, ec);
        if (d.mode == Mode::folder ? folder : !folder) {
            d.name = toUtf8(d.list.selected.filename());
            d.name_set = true;
        }
    }

    if (!d.error.empty()) TextColored(Theme::current().color("error", {1.0f, 0.3f, 0.3f, 1.0f}), "%s", d.error.c_str());

    // the options
    bool hidden = d.list.showHidden;
    if (Checkbox("Show hidden", &hidden)) d.list.showHidden = hidden;
    if (d.mode == Mode::save || d.mode == Mode::folder) {
        SameLine();
        if (Button((std::string(Icon::newFolder) + " New folder").c_str())) {
            d.naming = true;
            d.new_folder.clear();
            d.new_folder_error.clear();
        }
    }

    const char* ok = d.mode == Mode::save ? "Save" : d.mode == Mode::folder ? "Choose" : "Open";
    float buttons = CalcTextSize(ok).x + CalcTextSize("Cancel").x + style.FramePadding.x * 4.0f + style.ItemSpacing.x;
    bool filters = d.mode != Mode::folder;
    float combo = filters ? std::min(GetContentRegionAvail().x * 0.35f, GetFontSize() * 18.0f) : 0.0f;
    SetNextItemWidth(GetContentRegionAvail().x - combo - buttons - style.ItemSpacing.x * (filters ? 2.0f : 1.0f));
    if (IsWindowAppearing()) SetKeyboardFocusHere();
    if (d.name_set) {
        if (auto state = GetInputTextState(GetID("##name"))) state->ReloadUserBufAndSelectAll();
        d.name_set = false;
    }
    char buf[1024];
    auto n = std::min(d.name.size(), sizeof(buf) - 1);
    d.name.copy(buf, n);
    buf[n] = 0;
    const char* hint = d.mode == Mode::save ? "file name" : d.mode == Mode::folder ? "the folder shown, or a folder name or path" : "file name or path";
    if (InputTextWithHint("##name", hint, buf, sizeof(buf))) d.name = buf;
    bool enter = IsItemDeactivated() && (IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter));
    if (filters) {
        SameLine();
        SetNextItemWidth(combo);
        if (BeginCombo("##filter", d.filters[d.filter].label.c_str())) {
            for (int k = 0; k < (int)d.filters.size(); k++)
                if (Selectable(d.filters[k].label.c_str(), k == d.filter)) {
                    d.filter = k;
                    d.list.refresh();
                }
            EndCombo();
        }
    }
    SameLine();
    if ((Button(ok) || enter) && !chosen) chosen = accept(d, files);
    SameLine();
    bool modal_child = !d.replacing.empty() || d.naming;
    if (Button("Cancel") || (IsKeyPressed(ImGuiKey_Escape) && !modal_child && !d.list.editingPath())) open = false;

    // asked before saving over a file
    if (!d.replacing.empty()) OpenPopup("Replace?");
    if (BeginPopupModal("Replace?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        Text("'%s' is already there. Replace it?", toUtf8(d.replacing.filename()).c_str());
        if (Button("Replace")) {
            files = {d.replacing};
            chosen = true;
            d.replacing.clear();
            CloseCurrentPopup();
        }
        SameLine();
        if (Button("Cancel") || IsKeyPressed(ImGuiKey_Escape)) {
            d.replacing.clear();
            CloseCurrentPopup();
        }
        EndPopup();
    }

    // a new folder in the one shown
    if (d.naming) OpenPopup("New folder");
    if (BeginPopupModal("New folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        TextUnformatted("A new folder in");
        TextDisabled("%s", toUtf8(fs::path(d.list.dir).make_preferred()).c_str());
        char nb[256];
        auto nn = std::min(d.new_folder.size(), sizeof(nb) - 1);
        d.new_folder.copy(nb, nn);
        nb[nn] = 0;
        if (IsWindowAppearing()) SetKeyboardFocusHere();
        SetNextItemWidth(GetFontSize() * 20.0f);
        bool go = InputTextWithHint("##folder", "name", nb, sizeof(nb), ImGuiInputTextFlags_EnterReturnsTrue);
        d.new_folder = nb;
        if (!d.new_folder_error.empty()) TextColored(Theme::current().color("error", {1.0f, 0.3f, 0.3f, 1.0f}), "%s", d.new_folder_error.c_str());
        if (Button("Create") || go) {
            fs::path p = d.list.dir / fromUtf8(d.new_folder);
            if (d.new_folder.empty() || d.new_folder.find_first_of("/\\") != std::string::npos)
                d.new_folder_error = "Type a name without slashes";
            else if (fs::exists(p, ec))
                d.new_folder_error = "There's one by that name";
            else if (!fs::create_directory(p, ec))
                d.new_folder_error = ec ? ec.message() : "Can't make it";
            else {
                d.list.go(d.list.dir, p);
                d.name = d.mode == Mode::folder ? d.new_folder : d.name;
                d.name_set = true;
                d.naming = false;
                CloseCurrentPopup();
            }
        }
        SameLine();
        if (Button("Cancel") || IsKeyPressed(ImGuiKey_Escape)) {
            d.naming = false;
            CloseCurrentPopup();
        }
        EndPopup();
    }

    if (chosen) remember(files[0].parent_path());
    if (chosen || !open) CloseCurrentPopup();
    EndPopup();
    if (chosen || !open) dialogs.erase(key);
    return chosen;
}

}

bool browseButton(const char* id, std::string& path, const fs::path& base, const char* filters, const char* title)
{
    using namespace ImGui;

    PushID(id);
    // one dialog per button
    auto key = "file_dialog_" + std::to_string(GetID("dialog"));

    if (Button("...")) {
        std::error_code ec;
        fs::path current = path.empty() ? fs::path{} : fromUtf8(path);
        if (!current.empty() && current.is_relative()) current = base / current;
        fs::path dir = current.empty() ? base : current.parent_path();
        if (dir.empty() || !fs::is_directory(dir, ec)) dir = base;
        start(key, title, filters, dir, toUtf8(current.filename()), Mode::open);
    }
    SetItemTooltip("Choose a file");

    std::vector<fs::path> files;
    bool chosen = display(key, files);
    if (chosen) path = relative_if_inside(files[0], base);
    PopID();
    return chosen;
}

void openDialog(const char* id, const char* title, const char* filters, const fs::path& start_at, bool save)
{
    std::error_code ec;
    bool is_dir = fs::is_directory(start_at, ec);
    start(id, title, filters, is_dir ? start_at : start_at.parent_path(), is_dir ? std::string{} : toUtf8(start_at.filename()), save ? Mode::save : Mode::open);
}

void openFolderDialog(const char* id, const char* title, const fs::path& start_at)
{
    start(id, title, nullptr, start_at, {}, Mode::folder);
}

void openFilesDialog(const char* id, const char* title, const char* filters, const fs::path& start_at)
{
    start(id, title, filters, start_at, {}, Mode::files);
}

bool dialogResult(const char* id, fs::path& file)
{
    std::vector<fs::path> files;
    if (!display(id, files)) return false;
    file = files[0];
    return true;
}

bool dialogResults(const char* id, std::vector<fs::path>& files)
{
    return display(id, files);
}

void setRecentFolders(std::vector<fs::path> folders)
{
    recent = std::move(folders);
    if (recent.size() > max_recent) recent.resize(max_recent);
}

const std::vector<fs::path>& recentFolders()
{
    return recent;
}

void setDialogRoot(const std::filesystem::path& root, const std::string& name)
{
    dialog_root = root;
    root_name = name;
}

void setDialogUpload(std::function<void(const fs::path&)> u)
{
    upload = std::move(u);
}

fs::path dialogFolder()
{
    for (auto& [key, d] : dialogs)
        if (d->visible) return d->list.dir;
    return {};
}

bool dialogShowFile(const fs::path& file)
{
    for (auto& [key, dp] : dialogs) {
        auto& d = *dp;
        if (!d.visible) continue;
        // all files if the filter would hide it
        if (!matches(d.filters[d.filter], file))
            for (int k = 0; k < (int)d.filters.size(); k++)
                if (d.filters[k].extensions.empty()) d.filter = k;
        d.list.go(file.parent_path(), file);
        d.name = toUtf8(file.filename());
        d.name_set = true;
        d.error.clear();
        return true;
    }
    return false;
}

}
