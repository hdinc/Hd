// what Platform does without a window: paths

#include <Hd/Platform.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

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
#else
    CHECK(normalPath("/c/../tmp") == "/tmp");
#endif
    if (failures) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}
