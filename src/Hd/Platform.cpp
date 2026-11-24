#include <Hd/Platform.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#elif !defined(__EMSCRIPTEN__)
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <thread>
extern char** environ;
#endif

namespace fs = std::filesystem;

namespace Hd::Platform {

namespace {

    fs::path u8(const std::string& s) { return fs::path(std::u8string(s.begin(), s.end())); }

    std::string str(const fs::path& p)
    {
        auto u = p.u8string();
        return std::string(u.begin(), u.end());
    }

    std::string firstExisting(std::initializer_list<fs::path> paths)
    {
        std::error_code ec;
        for (const auto& p : paths)
            if (fs::is_regular_file(p, ec)) return str(p);
        return {};
    }

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
    // a command's output; empty when it fails
    std::string run(const std::string& cmd)
    {
        std::string out;
        if (FILE* f = popen(cmd.c_str(), "r")) {
            char b[4096];
            size_t n;
            while ((n = fread(b, 1, sizeof b, f)) > 0)
                out.append(b, n);
            if (pclose(f) != 0) out.clear();
        }
        return out;
    }

    // for a single-quoted shell argument
    std::string quote(const std::string& s)
    {
        std::string o = "'";
        for (char c : s)
            o += c == '\'' ? std::string("'\\''") : std::string(1, c);
        return o + "'";
    }

    std::vector<std::string> lines(const std::string& s)
    {
        std::vector<std::string> out;
        size_t a = 0;
        while (a < s.size()) {
            size_t b = s.find_first_of("|\n", a);
            if (b == std::string::npos) b = s.size();
            if (b > a) out.push_back(s.substr(a, b - a));
            a = b + 1;
        }
        return out;
    }

    // an executable of that name in a PATH folder
    bool onPath(const char* name)
    {
        const char* path = getenv("PATH");
        if (!path) return false;
        std::string all = path;
        for (size_t a = 0; a <= all.size();) {
            size_t b = std::min(all.find(':', a), all.size());
            if (b > a && access((all.substr(a, b - a) + "/" + name).c_str(), X_OK) == 0) return true;
            a = b + 1;
        }
        return false;
    }

    // the dialog program to run: kdialog first on KDE, zenity first
    // elsewhere; empty if neither is there
    std::string dialogProgram()
    {
        const char* desktop = getenv("XDG_CURRENT_DESKTOP");
        bool kde = desktop && strstr(desktop, "KDE");
        const char* order[] = {kde ? "kdialog" : "zenity", kde ? "zenity" : "kdialog"};
        for (const char* p : order)
            if (onPath(p)) return p;
        return {};
    }
#endif

}

#if defined(_WIN32)
namespace {
    // MSYS2, Git Bash and Cygwin write drives as /c/... or /cygdrive/c/...;
    // std::filesystem reads that as a folder c on the current drive. Paths
    // usually come converted, but not from every launcher or script.
    std::string fromMsys(const std::string& p)
    {
        std::string rest = p.rfind("/cygdrive/", 0) == 0 ? p.substr(9) : p;
        bool drive = rest.size() >= 2 && rest[0] == '/' && std::isalpha(static_cast<unsigned char>(rest[1]))
            && (rest.size() == 2 || rest[2] == '/' || rest[2] == '\\');
        if (!drive) return p;
        std::string root = std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(rest[1])))) + ":";
        std::error_code ec;
        // only a drive that exists: otherwise it may be a real folder
        if (!fs::exists(u8(root + "\\"), ec)) return p;
        return root + (rest.size() > 2 ? rest.substr(2) : "/");
    }
}
#endif

std::string normalPath(const std::string& path)
{
#if defined(_WIN32)
    std::string p = fromMsys(path);
#else
    const std::string& p = path;
#endif
    std::error_code ec;
    fs::path a = fs::absolute(u8(p), ec);
    if (ec) return p;
    fs::path c = fs::weakly_canonical(a, ec);
    return str(ec ? a.lexically_normal() : c);
}

#if defined(_WIN32)

namespace {
    std::string fromWide(const wchar_t* w) { return str(fs::path(w)); }

    fs::path windowsDir()
    {
        const wchar_t* w = _wgetenv(L"WINDIR");
        return fs::path(w ? w : L"C:\\Windows");
    }
}

std::vector<std::string> args(int, char**)
{
    std::vector<std::string> out;
    int n = 0;
    wchar_t** w = CommandLineToArgvW(GetCommandLineW(), &n);
    for (int i = 1; w && i < n; i++)
        out.push_back(fromWide(w[i]));
    if (w) LocalFree(w);
    return out;
}

bool hasFileDialog() { return true; }

std::vector<std::string> openFilesDialog(GLFWwindow* parent, const std::string& title, const std::vector<FileFilter>& filters, bool multiple)
{
    // "name\0patterns\0...\0\0"
    std::wstring filter;
    for (const auto& f : filters) {
        filter += u8(f.name).wstring();
        filter += L'\0';
        filter += u8(f.patterns).wstring();
        filter += L'\0';
    }
    filter += L"All files";
    filter += L'\0';
    filter += L"*.*";
    filter += L'\0';
    std::wstring wtitle = u8(title).wstring();
    std::vector<wchar_t> buf(32768, 0);
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof ofn;
    ofn.hwndOwner = parent ? glfwGetWin32Window(parent) : nullptr;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile = buf.data();
    ofn.nMaxFile = static_cast<DWORD>(buf.size());
    ofn.lpstrTitle = wtitle.empty() ? nullptr : wtitle.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | (multiple ? OFN_ALLOWMULTISELECT : 0);
    std::vector<std::string> out;
    if (!GetOpenFileNameW(&ofn)) return out;
    // one file: its path; several: the folder, then each name
    std::wstring first = buf.data();
    const wchar_t* p = buf.data() + first.size() + 1;
    if (!*p) {
        out.push_back(fromWide(first.c_str()));
        return out;
    }
    for (; *p; p += wcslen(p) + 1)
        out.push_back(str(fs::path(first) / p));
    return out;
}

std::string openFolderDialog(GLFWwindow* parent, const std::string& title, const std::string& start)
{
    // COM may already be set up on this thread (S_FALSE), or in another
    // mode: then it stays as it is
    HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    std::string out;
    IFileOpenDialog* dlg = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dlg)))) {
        DWORD opts = 0;
        dlg->GetOptions(&opts);
        dlg->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        std::wstring wtitle = u8(title).wstring();
        if (!wtitle.empty()) dlg->SetTitle(wtitle.c_str());
        if (!start.empty()) {
            std::wstring wstart = u8(start).make_preferred().wstring();
            IShellItem* folder = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(wstart.c_str(), nullptr, IID_PPV_ARGS(&folder)))) {
                dlg->SetFolder(folder);
                folder->Release();
            }
        }
        if (SUCCEEDED(dlg->Show(parent ? glfwGetWin32Window(parent) : nullptr))) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(dlg->GetResult(&item))) {
                PWSTR p = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &p))) out = fromWide(p);
                CoTaskMemFree(p);
                item->Release();
            }
        }
        dlg->Release();
    }
    if (SUCCEEDED(init)) CoUninitialize();
    return out;
}

bool openUri(const std::string& uri)
{
    std::wstring w = u8(uri).wstring();
    return reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", w.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
}

bool prefersDarkTheme()
{
    DWORD light = 1, size = sizeof light;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size) != ERROR_SUCCESS) return false;
    return light == 0;
}

void setDarkTitleBar(GLFWwindow* window, bool dark)
{
    if (!window) return;
    BOOL on = dark ? TRUE : FALSE;
    // DWMWA_USE_IMMERSIVE_DARK_MODE: 20, 19 before Windows 10 20H1
    HWND h = glfwGetWin32Window(window);
    if (FAILED(DwmSetWindowAttribute(h, 20, &on, sizeof on))) DwmSetWindowAttribute(h, 19, &on, sizeof on);
}

std::string systemUiFont(bool bold)
{
    fs::path f = windowsDir() / "Fonts";
    constexpr const char* faces[][2] = {
        {"segoeui.ttf", "segoeuib.ttf"},
        {"arial.ttf", "arialbd.ttf"},
        {"tahoma.ttf", "tahomabd.ttf"},
    };
    for (auto& [regular, heavy] : faces) {
        auto path = firstExisting({f / regular});
        if (!path.empty()) return bold ? firstExisting({f / heavy}) : path;
    }
    return {};
}

#elif defined(__EMSCRIPTEN__)

std::vector<std::string> args(int argc, char** argv) { return std::vector<std::string>(argv + 1, argv + argc); }

bool hasFileDialog() { return false; }

std::vector<std::string> openFilesDialog(GLFWwindow*, const std::string&, const std::vector<FileFilter>&, bool) { return {}; }

std::string openFolderDialog(GLFWwindow*, const std::string&, const std::string&) { return {}; }

bool openUri(const std::string&) { return false; }

bool prefersDarkTheme() { return false; }

void setDarkTitleBar(GLFWwindow*, bool) {}

std::string systemUiFont(bool) { return {}; }

#else

std::vector<std::string> args(int argc, char** argv) { return std::vector<std::string>(argv + 1, argv + argc); }

bool hasFileDialog() { return !dialogProgram().empty(); }

std::vector<std::string> openFilesDialog(GLFWwindow*, const std::string& title, const std::vector<FileFilter>& filters, bool multiple)
{
    // one program only: both exit with 1 when cancelled, so trying the
    // other after an empty answer would show a second dialog
    std::string program = dialogProgram();
    std::string cmd;
    if (program == "zenity") {
        // paths separated with |
        cmd = "zenity --file-selection --separator='|' --title=" + quote(title.empty() ? "Open" : title);
        if (multiple) cmd += " --multiple";
        for (const auto& f : filters) {
            std::string pats = f.patterns;
            for (char& c : pats)
                if (c == ';') c = ' ';
            cmd += " --file-filter=" + quote(f.name + " | " + pats);
        }
        cmd += " --file-filter=" + quote("All files | *");
    } else if (program == "kdialog") {
        // one path on a line
        std::string kfilter;
        for (const auto& f : filters) {
            std::string pats = f.patterns;
            for (char& c : pats)
                if (c == ';') c = ' ';
            kfilter += (kfilter.empty() ? "" : "\n") + f.name + " (" + pats + ")";
        }
        cmd = "kdialog --title " + quote(title.empty() ? "Open" : title) + " --getopenfilename . " + quote(kfilter.empty() ? "*" : kfilter);
        if (multiple) cmd += " --multiple --separate-output";
    } else
        return {};
    return lines(run(cmd + " 2>/dev/null"));
}

std::string openFolderDialog(GLFWwindow*, const std::string& title, const std::string& start)
{
    std::string program = dialogProgram();
    std::string t = quote(title.empty() ? "Choose a folder" : title);
    std::string cmd;
    if (program == "zenity")
        // a trailing slash starts inside the folder
        cmd = "zenity --file-selection --directory --title=" + t + (start.empty() ? "" : " --filename=" + quote(start + "/"));
    else if (program == "kdialog")
        cmd = "kdialog --title " + t + " --getexistingdirectory " + quote(start.empty() ? "." : start);
    else
        return {};
    auto out = lines(run(cmd + " 2>/dev/null"));
    return out.empty() ? std::string() : out[0];
}

bool openUri(const std::string& uri)
{
    // no shell: the URI is passed as one argument. Not waiting for it:
    // without a desktop environment xdg-open runs the program itself, until
    // it exits; a thread reaps it then.
    pid_t pid;
    char* argv[] = {const_cast<char*>("xdg-open"), const_cast<char*>(uri.c_str()), nullptr};
    if (posix_spawnp(&pid, "xdg-open", nullptr, nullptr, argv, environ) != 0) return false;
    std::thread([pid] { waitpid(pid, nullptr, 0); }).detach();
    return true;
}

void setDarkTitleBar(GLFWwindow*, bool) {}

bool prefersDarkTheme()
{
    // asking gsettings starts a process: ask now and then
    using Clock = std::chrono::steady_clock;
    static Clock::time_point last;
    static bool dark = false, known = false;
    if (known && Clock::now() - last < std::chrono::seconds(10)) return dark;
    last = Clock::now();
    known = true;
    if (const char* t = getenv("GTK_THEME"); t && strstr(t, ":dark")) return dark = true;
    std::string s = run("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null");
    if (s.find("dark") != std::string::npos) return dark = true;
    // KDE writes its colour scheme's name
    if (const char* home = getenv("HOME")) {
        std::ifstream kde(fs::path(home) / ".config" / "kdeglobals");
        std::string line;
        while (std::getline(kde, line))
            if (line.rfind("ColorScheme=", 0) == 0) return dark = line.find("Dark") != std::string::npos;
    }
    return dark = false;
}

std::string systemUiFont(bool bold)
{
    std::string f = run(bold ? "fc-match -f '%{file}' 'sans-serif:weight=bold' 2>/dev/null"
                             : "fc-match -f '%{file}' sans-serif 2>/dev/null");
    std::error_code ec;
    if (!f.empty() && fs::is_regular_file(f, ec)) return f;
    if (bold) return firstExisting({"/usr/share/fonts/noto/NotoSans-Bold.ttf", "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf", "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"});
    return firstExisting({"/usr/share/fonts/noto/NotoSans-Regular.ttf", "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf", "/usr/share/fonts/TTF/DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"});
}

#endif

}
