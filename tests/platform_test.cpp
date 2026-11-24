// what Platform does without a window: paths

#include <Hd/Platform.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
#include <filesystem>
#include <fstream>
#include <iterator>
#include <unistd.h>
#endif

static int failures = 0;
#define CHECK(c)                                                    \
    do {                                                            \
        if (!(c)) {                                                 \
            std::printf("line %d: %s failed\n", __LINE__, #c);      \
            failures++;                                             \
        }                                                           \
    } while (0)

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
namespace fs = std::filesystem;

static void writeScript(const fs::path& p, const std::string& body)
{
    std::ofstream(p) << "#!/bin/sh\n" << body;
    fs::permissions(p, fs::perms::owner_all);
}

static std::string readFile(const fs::path& p)
{
    std::ifstream in(p);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// openFilesDialog with fake zenity and kdialog programs on a PATH of their
// own: which one runs, what it is given, and how its answer is read
static void testDialogPrograms()
{
    using namespace Hd::Platform;
    fs::path dir = fs::temp_directory_path() / ("hd_platform_test_" + std::to_string(getpid()));
    fs::remove_all(dir);
    fs::create_directories(dir);
    const char* oldPath = std::getenv("PATH");
    std::string savedPath = oldPath ? oldPath : "";
    const char* oldDesktop = std::getenv("XDG_CURRENT_DESKTOP");
    std::string savedDesktop = oldDesktop ? oldDesktop : "";
    setenv("PATH", dir.c_str(), 1);
    std::vector<FileFilter> pdf{{"PDF documents", "*.pdf;*.PDF"}};
    fs::path zArgs = dir / "zenity.args", kArgs = dir / "kdialog.args";

    // neither: no dialog, and no answer
    CHECK(!hasFileDialog());
    CHECK(openFilesDialog(nullptr, "Open", pdf).empty());

    // kdialog only: one path on a line
    writeScript(dir / "kdialog", "printf '%s\\n' \"$@\" > '" + kArgs.string() + "'\nprintf '/a b.pdf\\n/c.pdf\\n'\n");
    CHECK(hasFileDialog());
    CHECK((openFilesDialog(nullptr, "Open", pdf) == std::vector<std::string>{"/a b.pdf", "/c.pdf"}));
    CHECK(readFile(kArgs).find("PDF documents (*.pdf *.PDF)\n") != std::string::npos);

    // both, outside KDE: zenity runs, paths separated with |
    writeScript(dir / "zenity", "printf '%s\\n' \"$@\" > '" + zArgs.string() + "'\nprintf '/x.pdf|/y z.pdf\\n'\n");
    setenv("XDG_CURRENT_DESKTOP", "GNOME", 1);
    fs::remove(kArgs);
    CHECK((openFilesDialog(nullptr, "Open", pdf) == std::vector<std::string>{"/x.pdf", "/y z.pdf"}));
    CHECK(readFile(zArgs).find("--file-filter=PDF documents | *.pdf *.PDF\n") != std::string::npos);
    CHECK(!fs::exists(kArgs));

    // cancelled (exit 1, no output): no answer, and kdialog isn't tried
    writeScript(dir / "zenity", "exit 1\n");
    CHECK(openFilesDialog(nullptr, "Open", pdf).empty());
    CHECK(!fs::exists(kArgs));

    // on KDE, kdialog first
    setenv("XDG_CURRENT_DESKTOP", "KDE", 1);
    CHECK(openFilesDialog(nullptr, "Open", pdf).size() == 2);
    CHECK(fs::exists(kArgs));

    setenv("PATH", savedPath.c_str(), 1);
    if (oldDesktop) setenv("XDG_CURRENT_DESKTOP", savedDesktop.c_str(), 1);
    else unsetenv("XDG_CURRENT_DESKTOP");
    fs::remove_all(dir);
}
#endif

int main()
{
    using Hd::Platform::normalPath;
#ifdef _WIN32
    // MSYS and Cygwin drive paths, on the system drive (which exists)
    const char* sd = std::getenv("SystemDrive");
    char d = sd && std::isalpha(static_cast<unsigned char>(sd[0])) ? static_cast<char>(std::toupper(static_cast<unsigned char>(sd[0]))) : 'C';
    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(d)));
    std::string root = std::string(1, d) + ":\\";
    CHECK(normalPath(std::string("/") + lower + "/Users/x/file.pdf") == root + "Users\\x\\file.pdf");
    CHECK(normalPath(std::string("/") + d + "/Users/x/file.pdf") == root + "Users\\x\\file.pdf");
    CHECK(normalPath(std::string("/cygdrive/") + lower + "/Users/x/file.pdf") == root + "Users\\x\\file.pdf");
    CHECK(normalPath(std::string("/") + lower) == root);
    CHECK(normalPath(root + "Users\\x\\..\\y.pdf") == root + "Users\\y.pdf");
    // not a drive: a longer first name stays a folder on the current drive
    std::string here = normalPath("/cc/file.pdf");
    CHECK(here.size() > 3 && here.substr(1, 6) == ":\\cc\\f");
    // a drive letter that doesn't exist stays as it is (a folder)
    std::string missing;
    for (char c = 'Z'; c > 'D'; c--) {
        std::string r = std::string(1, c) + ":\\";
        if (normalPath(std::string("/") + static_cast<char>(std::tolower(c)) + "/a") != r + "a") {
            missing = std::string(1, static_cast<char>(std::tolower(c)));
            break;
        }
    }
    if (!missing.empty()) {
        std::string p = normalPath("/" + missing + "/a");
        CHECK(p.find(":\\" + missing + "\\a") != std::string::npos);
    }
#elif !defined(__EMSCRIPTEN__)
    CHECK(normalPath("/c/../tmp") == "/tmp");
    testDialogPrograms();
#endif
    if (failures) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}
