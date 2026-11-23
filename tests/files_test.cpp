// file locks between processes and atomic replacement (Hd/Files.h)
//   hd_files_test DIR                     the tests, in an empty folder DIR
//   hd_files_test --try LOCK              exit 0 if LOCK can be taken at once, else 1
//   hd_files_test --take-and-exit LOCK    take LOCK and exit holding it

#include <Hd/Files.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#ifdef _WIN32
#include <share.h>
#else
#include <sys/wait.h>
#endif

static int failures = 0;
#define CHECK(c)                                                    \
    do {                                                            \
        if (!(c)) {                                                 \
            std::printf("line %d: %s failed\n", __LINE__, #c);      \
            failures++;                                             \
        }                                                           \
    } while (0)

namespace fs = std::filesystem;

static std::string self;

static std::string read(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    std::ostringstream s;
    s << in.rdbuf();
    return s.str();
}

// runs this program with arguments; returns its exit code
static int child(const std::string& mode, const std::string& lock)
{
    std::string cmd = "\"" + self + "\" " + mode + " \"" + lock + "\"";
#ifdef _WIN32
    cmd = "\"" + cmd + "\""; // cmd.exe drops the outer quotes
#endif
    int r = std::system(cmd.c_str());
#ifndef _WIN32
    if (r != -1) r = WEXITSTATUS(r);
#endif
    return r;
}

static size_t leftovers(const fs::path& dir)
{
    size_t n = 0;
    for (const auto& e : fs::directory_iterator(dir))
        if (e.path().filename().string().find(".tmp") != std::string::npos) n++;
    return n;
}

int main(int argc, char** argv)
{
    self = argv[0];
    if (argc == 3 && std::string(argv[1]) == "--try") return Hd::FileLock(argv[2], 0).locked() ? 0 : 1;
    if (argc == 3 && std::string(argv[1]) == "--take-and-exit") {
        auto* held = new Hd::FileLock(argv[2], 1); // never released: the exit must free it
        std::_Exit(held->locked() ? 0 : 1);
    }
    if (argc < 2) {
        std::printf("usage: hd_files_test DIR\n");
        return 2;
    }
    fs::path dir = argv[1];
    fs::remove_all(dir);
    fs::create_directories(dir);
    std::string file = (dir / "settings.ini").string();
    std::string lock = (dir / "settings.ini.lock").string();

    // replacement
    CHECK(Hd::replaceFile(file, "first\n"));
    CHECK(read(file) == "first\n");
    CHECK(Hd::replaceFile(file, "second, longer\n"));
    CHECK(read(file) == "second, longer\n");
    CHECK(Hd::replaceFile(file, ""));
    CHECK(read(file).empty());
    CHECK(Hd::replaceFile(file, "kept\n"));
    CHECK(leftovers(dir) == 0);
    // a missing folder fails and creates nothing
    CHECK(!Hd::replaceFile((dir / "missing" / "x.ini").string(), "x"));
    CHECK(!fs::exists(dir / "missing"));
#ifdef _WIN32
    {
        // a reader that doesn't share deletion blocks the move: the old file
        // stays readable and the temporary file goes
        FILE* reader = _wfsopen(fs::path(file).c_str(), L"rb", _SH_DENYNO);
        CHECK(reader != nullptr);
        CHECK(!Hd::replaceFile(file, "lost\n"));
        if (reader) fclose(reader);
        CHECK(read(file) == "kept\n");
        CHECK(leftovers(dir) == 0);
    }
#endif

    // locks, in this process
    {
        Hd::FileLock a(lock, 0);
        CHECK(a.locked());
        Hd::FileLock b(lock, 0.05);
        CHECK(!b.locked());
        a.unlock();
        Hd::FileLock c(lock, 0);
        CHECK(c.locked());
        Hd::FileLock moved(std::move(c));
        CHECK(moved.locked());
        CHECK(!c.locked());
    }
    // and with other processes
    {
        Hd::FileLock a(lock, 0);
        CHECK(a.locked());
        CHECK(child("--try", lock) == 1);
        a.unlock();
        CHECK(child("--try", lock) == 0);
    }
    // a process that exits holding the lock frees it
    CHECK(child("--take-and-exit", lock) == 0);
    CHECK(Hd::FileLock(lock, 1).locked());

    fs::remove_all(dir);
    if (failures) {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("all tests passed\n");
    return 0;
}
