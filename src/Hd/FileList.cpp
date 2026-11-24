#include <Hd/FileList.h>
#include <Hd/Icons.h>
#include <Hd/Platform.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iterator>
#include <mutex>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <knownfolders.h>
#include <objbase.h>
#include <shlobj.h>
#endif

#ifndef __EMSCRIPTEN__
#include <thread>

#include <GLFW/glfw3.h>
#endif

namespace Hd {

namespace fs = std::filesystem;

std::string toUtf8(const fs::path& p)
{
    auto s = p.u8string();
    return std::string(s.begin(), s.end());
}

fs::path fromUtf8(std::string_view s)
{
    return fs::path(std::u8string(s.begin(), s.end()));
}

bool naturalLess(const std::string& a, const std::string& b)
{
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (std::isdigit((unsigned char)a[i]) && std::isdigit((unsigned char)b[j])) {
            // the numbers without leading zeros, longer is larger
            while (i < a.size() && a[i] == '0') i++;
            while (j < b.size() && b[j] == '0') j++;
            size_t si = i, sj = j;
            while (i < a.size() && std::isdigit((unsigned char)a[i])) i++;
            while (j < b.size() && std::isdigit((unsigned char)b[j])) j++;
            if (i - si != j - sj) return i - si < j - sj;
            auto c = a.compare(si, i - si, b, sj, j - sj);
            if (c != 0) return c < 0;
            continue;
        }
        int ca = std::tolower((unsigned char)a[i]), cb = std::tolower((unsigned char)b[j]);
        if (ca != cb) return ca < cb;
        i++;
        j++;
    }
    if (a.size() - i != b.size() - j) return a.size() - i < b.size() - j;
    // only case differs: a fixed order
    return a < b;
}

bool isHidden(const fs::path& p)
{
    auto name = toUtf8(p.filename());
    if (!name.empty() && name[0] == '.' && name != "." && name != "..") return true;
#ifdef _WIN32
    DWORD a = GetFileAttributesW(p.c_str());
    if (a != INVALID_FILE_ATTRIBUTES && (a & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))) return true;
#endif
    return false;
}

namespace {

    std::string lowerAscii(std::string s)
    {
        for (auto& c : s)
            c = (char)std::tolower((unsigned char)c);
        return s;
    }

    std::string env(const char* name)
    {
#ifdef _WIN32
        std::wstring w(name, name + std::char_traits<char>::length(name));
        const wchar_t* v = _wgetenv(w.c_str());
        return v ? toUtf8(fs::path(v)) : std::string();
#else
        const char* v = std::getenv(name);
        return v ? v : "";
#endif
    }

    fs::path homeFolder()
    {
#ifdef _WIN32
        auto h = env("USERPROFILE");
#else
        auto h = env("HOME");
#endif
        std::error_code ec;
        return h.empty() ? fs::current_path(ec) : fromUtf8(h);
    }

    bool isSeparator(char c)
    {
#ifdef _WIN32
        return c == '/' || c == '\\';
#else
        return c == '/';
#endif
    }

    // a file time as local "2026-10-27 23:10"
    std::string formatTime(fs::file_time_type t)
    {
        using namespace std::chrono;
        // file_clock's epoch isn't system_clock's everywhere
        auto sys = time_point_cast<system_clock::duration>(t - fs::file_time_type::clock::now() + system_clock::now());
        std::time_t tt = system_clock::to_time_t(sys);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm);
        return buf;
    }

}

std::vector<std::pair<std::string, fs::path>> parseUserDirs(std::string_view text, const fs::path& home)
{
    // the folders file managers list, in their order; Templates and Public
    // are left out
    static const char* keys[] = {"DESKTOP", "DOCUMENTS", "DOWNLOAD", "MUSIC", "PICTURES", "VIDEOS"};
    std::vector<fs::path> found(std::size(keys));
    size_t pos = 0;
    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string_view::npos) end = text.size();
        std::string_view line = text.substr(pos, end - pos);
        pos = end + 1;
        // XDG_DOCUMENTS_DIR="$HOME/Documents"
        if (!line.starts_with("XDG_")) continue;
        auto eq = line.find('=');
        if (eq == std::string_view::npos || !line.substr(0, eq).ends_with("_DIR")) continue;
        std::string_view key = line.substr(4, eq - 4 - 4);
        std::string_view value = line.substr(eq + 1);
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) value.remove_suffix(1);
        if (value.size() < 2 || value.front() != '"' || value.back() != '"') continue;
        value = value.substr(1, value.size() - 2);
        fs::path p;
        if (value.starts_with("$HOME")) {
            auto rest = value.substr(5);
            while (!rest.empty() && rest.front() == '/') rest.remove_prefix(1);
            if (rest.empty()) continue; // a folder turned off: $HOME itself
            p = home / fromUtf8(rest);
        } else if (value.starts_with("/")) {
            p = fromUtf8(value);
        } else {
            continue;
        }
        for (size_t k = 0; k < std::size(keys); k++)
            if (key == keys[k]) found[k] = p.lexically_normal();
    }
    std::vector<std::pair<std::string, fs::path>> out;
    for (auto& p : found)
        if (!p.empty() && p != home) out.push_back({toUtf8(p.filename()), p});
    return out;
}

std::vector<Place> places()
{
    std::vector<Place> out;
    std::error_code ec;
    auto add = [&](std::string name, const fs::path& p, const char* icon, bool drive = false) {
        if (p.empty() || !fs::is_directory(p, ec)) return;
        for (auto& q : out)
            if (q.path == p) return;
        out.push_back({std::move(name), p, icon, drive});
    };
#if defined(_WIN32)
    add("Home", homeFolder(), Icon::home);
    struct Known {
        const KNOWNFOLDERID* id;
        const char* name;
    };
    const Known known[] = {{&FOLDERID_Desktop, "Desktop"}, {&FOLDERID_Documents, "Documents"}, {&FOLDERID_Downloads, "Downloads"},
        {&FOLDERID_Music, "Music"}, {&FOLDERID_Pictures, "Pictures"}, {&FOLDERID_Videos, "Videos"}};
    for (auto& k : known) {
        PWSTR w = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(*k.id, 0, nullptr, &w))) add(k.name, fs::path(w), Icon::folder);
        CoTaskMemFree(w);
    }
    DWORD mask = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (!(mask & (1u << i))) continue;
        wchar_t root[] = {wchar_t(L'A' + i), L':', L'\\', 0};
        std::string letter = std::string(1, char('A' + i)) + ":";
        UINT type = GetDriveTypeW(root);
        std::string label;
        // asking a network or optical drive for its label can take long
        if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE) {
            wchar_t name[MAX_PATH + 1] = {};
            if (GetVolumeInformationW(root, name, MAX_PATH + 1, nullptr, nullptr, nullptr, nullptr, 0)) label = toUtf8(fs::path(name));
        }
        if (label.empty())
            label = type == DRIVE_REMOTE ? "Network Drive" : type == DRIVE_CDROM ? "CD Drive" : type == DRIVE_REMOVABLE ? "Removable Disk" : "Local Disk";
        // not is_directory: an empty card reader or CD drive still shows
        out.push_back({label + " (" + letter + ")", fs::path(root), Icon::folder, true});
    }
#else
    fs::path home = homeFolder();
    add("Home", home, Icon::home);
    std::string config = env("XDG_CONFIG_HOME");
    fs::path dirsFile = (config.empty() ? home / ".config" : fromUtf8(config)) / "user-dirs.dirs";
    std::ifstream in(dirsFile);
    if (in) {
        std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        for (auto& [name, p] : parseUserDirs(text, home))
            add(name, p, Icon::folder);
    } else {
        for (const char* name : {"Desktop", "Documents", "Downloads", "Music", "Pictures", "Videos"})
            add(name, home / name, Icon::folder);
    }
    add("File System", "/", Icon::folder, true);
#endif
    return out;
}

fs::path typedPath(std::string_view text, const fs::path& dir, const fs::path& home)
{
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.remove_prefix(1);
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r' || text.back() == '\n')) text.remove_suffix(1);
    if (text.empty()) return dir;
    fs::path p;
    if (text == "~" || (text.size() > 1 && text[0] == '~' && isSeparator(text[1]))) {
        p = home / fromUtf8(text.substr(std::min<size_t>(2, text.size())));
    } else {
#ifdef _WIN32
        // "C:" alone is that drive's root, not its current folder
        if (text.size() == 2 && std::isalpha((unsigned char)text[0]) && text[1] == ':') return fromUtf8(std::string(text) + "\\");
        // an MSYS drive path (/c/x): normalPath knows them
        if (text[0] == '/' && (text.size() < 2 || text[1] != '/')) return fromUtf8(Platform::normalPath(std::string(text))).lexically_normal();
        // a UNC path (\\server\share): libstdc++ doesn't know their root, and
        // lexically_normal would merge the two leading separators
        if (text.size() > 2 && isSeparator(text[0]) && isSeparator(text[1])) {
            std::string unc(text);
            for (auto& c : unc)
                if (c == '/') c = '\\';
            while (unc.size() > 2 && unc.back() == '\\') unc.pop_back();
            return fromUtf8(unc);
        }
#endif
        p = fromUtf8(text);
        if (p.is_relative()) {
            // "\x" on Windows: the current drive's root
            p = p.has_root_directory() ? dir.root_name() / p : dir / p;
        }
    }
    auto n = p.lexically_normal();
    // "a/b/" as "a/b", but keep a root's separator
    if (!n.has_filename() && n.has_relative_path()) n = n.parent_path();
    return n;
}

std::string completeFolder(std::string_view text, const fs::path& dir, const fs::path& home)
{
    // the folder typed so far and the start of a name in it
    size_t cut = text.size();
    while (cut > 0 && !isSeparator(text[cut - 1])) cut--;
    std::string_view head = text.substr(0, cut), prefix = text.substr(cut);
    fs::path parent = head.empty() ? dir : typedPath(head, dir, home);
    std::string want = lowerAscii(std::string(prefix));
    std::vector<std::string> names;
    std::error_code ec;
    for (auto it = fs::directory_iterator(parent, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
        std::error_code ignored;
        if (!it->is_directory(ignored)) continue;
        std::string name = toUtf8(it->path().filename());
        if (isHidden(it->path()) && !prefix.starts_with(".")) continue;
        if (lowerAscii(name).starts_with(want)) names.push_back(name);
    }
    if (names.empty()) return std::string(text);
    std::sort(names.begin(), names.end(), naturalLess);
    // as far as all of them agree, whole characters only
    std::string common = names[0];
    for (auto& n : names) {
        size_t k = 0;
        while (k < common.size() && k < n.size() && std::tolower((unsigned char)common[k]) == std::tolower((unsigned char)n[k])) k++;
        common.resize(k);
    }
    while (!common.empty() && (common.size() < names[0].size()) && ((unsigned char)names[0][common.size()] & 0xC0) == 0x80) common.pop_back();
    if (common.size() < prefix.size()) return std::string(text);
    std::string out = std::string(head) + common;
    if (names.size() == 1) out += '/';
    return out;
}

std::string formatSize(uintmax_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB"};
    double v = (double)bytes;
    int u = 0;
    while (v >= 1024.0 && u < 3) {
        v /= 1024.0;
        u++;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), u == 0 ? "%.0f %s" : "%.1f %s", v, units[u]);
    return buf;
}

// a folder read on the worker thread; the UI thread takes the result
struct FileList::Listing {
    fs::path dir;
    std::mutex m;
    bool done = false;
    std::vector<entry> entries;
    std::string error;

    void run()
    {
        std::vector<entry> out;
        std::error_code ec;
        for (auto it = fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
            entry e;
            e.path = it->path();
            e.name = toUtf8(e.path.filename());
            if (e.name.empty()) continue;
            std::error_code ignored;
            e.folder = it->is_directory(ignored);
            e.hidden = isHidden(e.path);
            if (!e.folder) e.size = it->file_size(ignored);
            auto t = it->last_write_time(ignored);
            if (!ignored) e.date = formatTime(t);
            out.push_back(std::move(e));
        }
        std::lock_guard lk(m);
        entries = std::move(out);
        if (ec) error = ec.message();
        done = true;
    }
};

FileList::FileList() = default;
FileList::~FileList() = default; // a read in progress finishes on its own

fs::path FileList::userHome() const { return home.empty() ? homeFolder() : home; }

void FileList::go(const fs::path& folder, const fs::path& select)
{
    fs::path next = folder;
    // "a/b/" (a typed folder) as "a/b": its parent and the path bar's parts
    // need the last name
    if (!next.has_filename() && next.has_relative_path()) next = next.parent_path();
    if (next != dir) {
        all.clear();
        entries.clear();
        listed.clear();
        error.clear();
    }
    dir = next;
    selected = select;
    selection.clear();
    if (!select.empty()) selection.push_back(select);
    anchor = select;
    scrollToSelected = !select.empty();
    stale = true;
}

void FileList::startRead()
{
    // one read at a time per folder; one for another folder is left to finish
    if (reading && reading->dir == dir) return;
    auto job = std::make_shared<Listing>();
    job->dir = dir;
    reading = job;
    stale = false;
    readAt = ImGui::GetTime();
#ifdef __EMSCRIPTEN__
    job->run();
#else
    std::thread([job] {
        job->run();
        // a frame to show it, for programs that wait for events
        glfwPostEmptyEvent();
    }).detach();
#endif
}

bool FileList::takeRead()
{
    if (!reading) return false;
    std::lock_guard lk(reading->m);
    if (!reading->done) return false;
    if (reading->dir == dir) {
        all = std::move(reading->entries);
        error = reading->error;
        listed = dir;
        filter();
        // the selection only while it's still there
        auto there = [&](const fs::path& p) { return std::any_of(entries.begin(), entries.end(), [&](auto& e) { return e.path == p; }); };
        if (!selected.empty() && selected.parent_path() == dir && !there(selected)) selected.clear();
        std::erase_if(selection, [&](const fs::path& p) { return p.parent_path() == dir && !there(p); });
    }
    auto done = std::move(reading);
    return true;
}

void FileList::filter()
{
    entries.clear();
    for (auto& e : all) {
        if (e.hidden && !showHidden) continue;
        if (!e.folder && show && !show(e.path)) continue;
        entries.push_back(e);
    }
    lastHidden = showHidden;
    sort();
}

void FileList::sort()
{
    std::stable_sort(entries.begin(), entries.end(), [&](const entry& a, const entry& b) {
        if (a.folder != b.folder) return a.folder;
        auto less = [&](const entry& x, const entry& y) {
            switch (sortColumn) {
            case 1:
                if (x.size != y.size) return x.size < y.size;
                break;
            case 2:
                if (x.date != y.date) return x.date < y.date;
                break;
            }
            return naturalLess(x.name, y.name);
        };
        return descending ? less(b, a) : less(a, b);
    });
}

void FileList::editPath()
{
    editing = true;
    focusEdit = true;
    editText = toUtf8(fs::path(dir).make_preferred());
}

void FileList::pathBar()
{
    using namespace ImGui;

    if (!editing && IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !GetIO().WantTextInput && IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_L)) editPath();

    if (editing) {
        // the path as text: Tab completes a folder name, Enter goes there
        SetNextItemWidth(-FLT_MIN);
        if (focusEdit) {
            SetKeyboardFocusHere();
            focusEdit = false;
        }
        char buf[2048];
        auto n = std::min(editText.size(), sizeof(buf) - 1);
        editText.copy(buf, n);
        buf[n] = 0;
        struct Ctx {
            FileList* self;
        } ctx{this};
        bool enter = InputText("##path", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion,
            [](ImGuiInputTextCallbackData* data) {
                auto* c = static_cast<Ctx*>(data->UserData);
                std::string done = completeFolder(std::string_view(data->Buf, data->BufTextLen), c->self->dir, c->self->userHome());
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, done.c_str());
                return 0;
            },
            &ctx);
        editText = buf;
        if (enter) {
            fs::path p = typedPath(editText, dir, userHome());
            std::error_code ec;
            if (fs::is_directory(p, ec)) {
                go(p);
                editing = false;
            } else if (fs::exists(p, ec)) {
                go(p.parent_path(), p);
                editing = false;
            } else {
                // stays open to be corrected
                focusEdit = true;
            }
        } else if (IsItemDeactivated() || IsKeyPressed(ImGuiKey_Escape)) {
            editing = false;
        }
        SetItemTooltip("Type a folder; Tab completes its name, Enter goes there");
        return;
    }

    // from dir up to root (or the top)
    std::vector<fs::path> parts;
    for (auto p = dir; !p.empty(); p = p.parent_path()) {
        parts.push_back(p);
        if (p == root || p == p.parent_path()) break;
    }
    std::reverse(parts.begin(), parts.end());

    auto label = [](const fs::path& p) {
        auto name = toUtf8(p.filename());
        return name.empty() ? toUtf8(fs::path(p).make_preferred()) : name;
    };
    // the folders nearest to dir that fit, the ones before them behind "...";
    // a little room at the end to click for typing
    auto& style = GetStyle();
    float separator = CalcTextSize("/").x + 4.0f;
    auto button = [&](const std::string& s) { return CalcTextSize(s.c_str()).x + style.FramePadding.x * 2.0f; };
    float avail = GetContentRegionAvail().x - GetFontSize() * 2.0f;
    float width = 0.0f;
    size_t first = parts.size();
    while (first > 0) {
        float w = button(label(parts[first - 1])) + (first < parts.size() ? separator : 0.0f);
        // room for "..." unless it's the last one
        float more = first > 1 ? button("...") + separator : 0.0f;
        if (width + w + more > avail && first < parts.size()) break;
        width += w;
        first--;
    }

    if (first > 0) {
        if (SmallButton("...")) go(parts[first - 1], parts[first]);
        SetItemTooltip("%s", toUtf8(parts[first - 1]).c_str());
    }
    for (size_t k = first; k < parts.size(); k++) {
        if (k > 0 || first > 0) {
            SameLine(0, 2);
            TextUnformatted("/");
            SameLine(0, 2);
        }
        PushID((int)k);
        if (SmallButton(label(parts[k]).c_str()) && parts[k] != dir) go(parts[k], k + 1 < parts.size() ? parts[k + 1] : fs::path{});
        PopID();
    }
    // the rest of the line: a click types a path
    SameLine();
    float rest = GetContentRegionAvail().x;
    if (rest > 1.0f) {
        if (InvisibleButton("##edit", {rest, GetFrameHeight()})) editPath();
        if (IsItemHovered()) SetMouseCursor(ImGuiMouseCursor_TextInput);
        SetItemTooltip("Click to type a path (Ctrl+L)");
    } else {
        NewLine();
    }
}

void FileList::select(const fs::path& p, bool ctrl, bool shift)
{
    selected = p;
    if (!multiple || (!ctrl && !shift) || anchor.empty()) {
        selection = {p};
        anchor = p;
        return;
    }
    if (ctrl) {
        auto it = std::find(selection.begin(), selection.end(), p);
        if (it != selection.end())
            selection.erase(it);
        else
            selection.push_back(p);
        anchor = p;
        return;
    }
    // shift: from the anchor to p, in the order shown
    auto pos = [&](const fs::path& q) {
        for (size_t i = 0; i < entries.size(); i++)
            if (entries[i].path == q) return (long)i;
        return -1L;
    };
    long a = pos(anchor), b = pos(p);
    if (a < 0 || b < 0) {
        selection = {p};
        anchor = p;
        return;
    }
    selection.clear();
    for (long i = std::min(a, b); i <= std::max(a, b); i++)
        selection.push_back(entries[(size_t)i].path);
}

void FileList::keys(fs::path& into, fs::path& from, fs::path& activated, bool& chosen)
{
    using namespace ImGui;
    auto& io = GetIO();
    if (io.WantTextInput || !IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) return;

    auto index = [&]() -> long {
        for (size_t i = 0; i < entries.size(); i++)
            if (entries[i].path == selected) return (long)i;
        return -1;
    };
    auto moveTo = [&](long i) {
        if (entries.empty()) return;
        i = std::clamp(i, 0L, (long)entries.size() - 1);
        select(entries[(size_t)i].path, false, io.KeyShift);
        scrollToSelected = true;
    };
    if (IsKeyPressed(ImGuiKey_DownArrow)) moveTo(index() + 1);
    if (IsKeyPressed(ImGuiKey_UpArrow)) moveTo(index() < 0 ? 0 : index() - 1);
    if (IsKeyPressed(ImGuiKey_Home)) moveTo(0);
    if (IsKeyPressed(ImGuiKey_End)) moveTo((long)entries.size() - 1);
    if (IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter)) {
        long i = index();
        if (i >= 0) {
            auto& e = entries[(size_t)i];
            if (e.folder)
                into = e.path;
            else {
                activated = e.path;
                chosen = true;
            }
        }
    }
    bool up = IsKeyPressed(ImGuiKey_Backspace) || (io.KeyAlt && IsKeyPressed(ImGuiKey_UpArrow));
    if (up && dir != root && dir.has_parent_path() && dir.parent_path() != dir) {
        into = dir.parent_path();
        from = dir;
    }

    // type-ahead: the first name starting with what was typed in the last
    // second
    if (!io.InputQueueCharacters.empty() && !io.KeyCtrl && !io.KeyAlt) {
        double now = GetTime();
        if (now - typedAt > 1.0) typed.clear();
        typedAt = now;
        for (ImWchar c : io.InputQueueCharacters) {
            if (c < 32) continue;
            char buf[8];
            int n = ImTextCharToUtf8(buf, c);
            typed.append(buf, (size_t)n);
        }
        std::string want = lowerAscii(typed);
        for (auto& e : entries)
            if (lowerAscii(e.name).starts_with(want)) {
                select(e.path, false, false);
                scrollToSelected = true;
                break;
            }
    }
}

bool FileList::table(fs::path& activated, float height)
{
    using namespace ImGui;

    if (stale || dir != listed || GetTime() - readAt > 1.0) startRead();
    takeRead();
    if (showHidden != lastHidden) filter();

    bool chosen = false;
    auto flags = ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV
        | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit;
    if (!BeginTable("files", 3, flags, {0.0f, height})) return false;
    TableSetupScrollFreeze(0, 1);
    TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort);
    TableSetupColumn("Size", ImGuiTableColumnFlags_PreferSortDescending);
    TableSetupColumn("Modified", ImGuiTableColumnFlags_PreferSortDescending);
    // the date only where it leaves room for the names
    TableSetColumnEnabled(2, GetContentRegionAvail().x > GetFontSize() * 24.0f);
    TableHeadersRow();

    if (auto specs = TableGetSortSpecs(); specs && specs->SpecsDirty) {
        if (specs->SpecsCount > 0) {
            sortColumn = specs->Specs[0].ColumnIndex;
            descending = specs->Specs[0].SortDirection == ImGuiSortDirection_Descending;
        }
        sort();
        specs->SpecsDirty = false;
    }

    // a destination while drawing, taken after the table
    fs::path into;
    fs::path from;
    keys(into, from, activated, chosen);

    // where the names start in their column, after the wider icon
    float name_x = std::max(CalcTextSize(Icon::folder).x, CalcTextSize(Icon::file).x) + GetStyle().ItemInnerSpacing.x;

    if (dir != root && dir.has_parent_path() && dir.parent_path() != dir) {
        TableNextRow();
        TableNextColumn();
        auto at = GetCursorScreenPos();
        if (Selectable("##up", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) && IsMouseDoubleClicked(0)) {
            into = dir.parent_path();
            from = dir;
        }
        SetItemTooltip("Up one folder (Backspace)");
        SameLine();
        SetCursorScreenPos({at.x + name_x, at.y});
        TextUnformatted("..");
    }

    auto& io = GetIO();
    for (auto& e : entries) {
        TableNextRow();
        TableNextColumn();
        PushID(e.name.c_str());
        bool is_selected = multiple ? std::find(selection.begin(), selection.end(), e.path) != selection.end() : e.path == selected;
        auto at = GetCursorScreenPos();
        if (Selectable("##entry", is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
            select(e.path, io.KeyCtrl, io.KeyShift);
            if (IsMouseDoubleClicked(0)) {
                if (e.folder) {
                    into = e.path;
                } else {
                    activated = e.path;
                    chosen = true;
                }
            }
        }
        if (e.path == selected && scrollToSelected) {
            SetScrollHereY();
            scrollToSelected = false;
        }
        // the names line up whatever the icon's width
        SameLine();
        SetCursorScreenPos(at);
        if (e.hidden) BeginDisabled();
        TextUnformatted(e.folder ? Icon::folder : Icon::file);
        SameLine();
        SetCursorScreenPos({at.x + name_x, at.y});
        TextUnformatted(e.name.c_str());
        if (e.hidden) EndDisabled();
        PopID();
        TableNextColumn();
        if (!e.folder) TextUnformatted(formatSize(e.size).c_str());
        TableNextColumn();
        TextUnformatted(e.date.c_str());
    }
    if (entries.empty()) {
        TableNextRow();
        TableNextColumn();
        if (!error.empty())
            TextDisabled("%s", error.c_str());
        else
            TextDisabled(listed != dir ? "Reading..." : "empty");
    }
    EndTable();

    if (!into.empty()) go(into, from);
    return chosen;
}

}
