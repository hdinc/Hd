// what file dialogs need from the system, without a window: UTF-8 paths,
// natural order, hidden files, places, user dirs, typed paths, completion

#include <Hd/FileList.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

static int failures = 0;
#define CHECK(c)                                                    \
    do {                                                            \
        if (!(c)) {                                                 \
            std::printf("line %d: %s failed\n", __LINE__, #c);      \
            failures++;                                             \
        }                                                           \
    } while (0)

int main()
{
    using namespace Hd;

    // UTF-8 both ways, also where path::string() would be the ANSI code page
    std::string name = "\xe6\x9d\xb1\xe4\xba\xac Caf\xc3\xa9 \xc5\x9fiir"; // 東京 Café şiir
    CHECK(toUtf8(fromUtf8(name)) == name);
    CHECK(toUtf8(fromUtf8("a/" + name).filename()) == name);

    // natural order, case ignored
    CHECK(naturalLess("file2", "file10"));
    CHECK(!naturalLess("file10", "file2"));
    CHECK(naturalLess("a", "B"));
    CHECK(naturalLess("x01", "x2"));

    // user-dirs.dirs: $HOME replaced, turned off ones ($HOME itself) and
    // others left out, in file managers' order
    fs::path home = fromUtf8("/home/u");
    auto dirs = parseUserDirs("# comment\n"
                              "XDG_DOWNLOAD_DIR=\"$HOME/\xc4\xb0ndirilenler\"\n" // İndirilenler
                              "XDG_DOCUMENTS_DIR=\"$HOME/Belgeler\"\r\n"
                              "XDG_DESKTOP_DIR=\"$HOME/\"\n"
                              "XDG_TEMPLATES_DIR=\"$HOME/Templates\"\n"
                              "XDG_MUSIC_DIR=\"/data/music\"\n"
                              "broken line\n",
        home);
    CHECK(dirs.size() == 3);
    if (dirs.size() == 3) {
        CHECK(dirs[0].first == "Belgeler");
        CHECK(dirs[0].second == home / "Belgeler");
        CHECK(dirs[1].first == "\xc4\xb0ndirilenler");
        CHECK(dirs[2].second == fromUtf8("/data/music"));
    }

    // a scratch tree for typed paths, completion and hidden files
    fs::path tmp = fs::temp_directory_path() / ("hd_filelist_test_" + std::to_string(
#ifdef _WIN32
                                                    GetCurrentProcessId()
#else
                                                    getpid()
#endif
                                                        ));
    fs::remove_all(tmp);
    fs::create_directories(tmp / "Books" / "Old");
    fs::create_directories(tmp / "Bookmarks");
    fs::create_directories(tmp / fromUtf8(name));
    fs::create_directories(tmp / ".cache");
    std::ofstream(tmp / "Bootstrap.txt") << "a file, not a folder";

    // typed paths
    CHECK(typedPath("", tmp, home) == tmp);
    CHECK(typedPath("Books", tmp, home) == (tmp / "Books").lexically_normal());
    CHECK(typedPath("  Books/Old/  ", tmp, home) == (tmp / "Books" / "Old").lexically_normal());
    CHECK(typedPath("Books/../Bookmarks", tmp, home) == (tmp / "Bookmarks").lexically_normal());
    CHECK(typedPath("~", tmp, home) == home.lexically_normal());
    CHECK(typedPath("~/x", tmp, home) == (home / "x").lexically_normal());
#ifdef _WIN32
    CHECK(typedPath("C:", tmp, home) == fs::path(L"C:\\"));
    CHECK(typedPath("D:\\a\\..\\b", tmp, home) == fs::path(L"D:\\b"));
    CHECK(typedPath("\\\\server\\share\\x", tmp, home) == fs::path(L"\\\\server\\share\\x"));
    // MSYS drive paths: /c/... is drive C (it exists)
    CHECK(typedPath("/c/Windows", tmp, home) == fs::path(L"C:\\Windows"));
#else
    CHECK(typedPath("/usr/../etc", tmp, home) == fs::path("/etc"));
#endif

    // completion: as far as the folders agree, the folders' own case, a
    // separator after a single match; files and hidden folders aren't
    // offered
    std::string t = toUtf8(tmp) + "/";
    CHECK(completeFolder(t + "boo", tmp, home) == t + "Book");
    CHECK(completeFolder(t + "books", tmp, home) == t + "Books/");
    CHECK(completeFolder("Books/o", tmp, home) == "Books/Old/");
    CHECK(completeFolder("\xe6\x9d\xb1", tmp, home) == name + "/");
    CHECK(completeFolder("zz", tmp, home) == "zz");
    CHECK(completeFolder("Boot", tmp, home) == "Boot");
    CHECK(completeFolder(".ca", tmp, home) == ".cache/");

    // hidden
    CHECK(isHidden(tmp / ".cache"));
    CHECK(!isHidden(tmp / "Books"));
#ifdef _WIN32
    SetFileAttributesW((tmp / "Books").c_str(), FILE_ATTRIBUTE_HIDDEN);
    CHECK(isHidden(tmp / "Books"));
    SetFileAttributesW((tmp / "Books").c_str(), FILE_ATTRIBUTE_NORMAL);
#endif

    // places: home first, then folders that exist, the drives or the root last
    auto ps = places();
    CHECK(!ps.empty());
    if (!ps.empty()) CHECK(ps[0].name == "Home");
    for (auto& p : ps)
        if (!p.drive) CHECK(fs::is_directory(p.path));
    CHECK(!ps.empty() && ps.back().drive);

    fs::remove_all(tmp);
    if (failures) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}
