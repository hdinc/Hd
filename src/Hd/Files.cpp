#include <Hd/Files.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif !defined(__EMSCRIPTEN__)
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

#ifdef __EMSCRIPTEN__
#include <cstdio>
#include <fstream>
#endif

namespace Hd {

namespace {

namespace fs = std::filesystem;

fs::path u8(const std::string& s) { return fs::path(std::u8string(s.begin(), s.end())); }

// a name for the temporary file no other process uses
std::string tempName(const std::string& path)
{
#ifdef _WIN32
    unsigned long pid = GetCurrentProcessId();
#elif defined(__EMSCRIPTEN__)
    unsigned long pid = 0;
#else
    unsigned long pid = static_cast<unsigned long>(getpid());
#endif
    auto t = std::chrono::steady_clock::now().time_since_epoch().count();
    return path + ".tmp" + std::to_string(pid) + "-" + std::to_string(t);
}

} // namespace

FileLock::FileLock(const std::string& path, double timeout)
{
    auto until = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout);
#ifdef _WIN32
    HANDLE h = CreateFileW(u8(path).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    mHandle = reinterpret_cast<intptr_t>(h);
    for (;;) {
        OVERLAPPED ov = {};
        if (LockFileEx(h, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &ov)) {
            mLocked = true;
            return;
        }
        if (std::chrono::steady_clock::now() >= until) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#elif !defined(__EMSCRIPTEN__)
    int fd = open(path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0644);
    if (fd < 0) return;
    mHandle = fd;
    for (;;) {
        // flock locks belong to the open file, so two FileLocks conflict
        // even in one process
        if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
            mLocked = true;
            return;
        }
        if (std::chrono::steady_clock::now() >= until) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#else
    (void)path;
    (void)until;
    mLocked = true;
    return;
#endif
    unlock();
}

FileLock::FileLock(FileLock&& o) noexcept
    : mHandle(o.mHandle)
    , mLocked(o.mLocked)
{
    o.mHandle = -1;
    o.mLocked = false;
}

FileLock& FileLock::operator=(FileLock&& o) noexcept
{
    if (this != &o) {
        unlock();
        mHandle = o.mHandle;
        mLocked = o.mLocked;
        o.mHandle = -1;
        o.mLocked = false;
    }
    return *this;
}

void FileLock::unlock()
{
#ifdef _WIN32
    if (mHandle != -1) {
        HANDLE h = reinterpret_cast<HANDLE>(mHandle);
        if (mLocked) {
            OVERLAPPED ov = {};
            UnlockFileEx(h, 0, 1, 0, &ov);
        }
        CloseHandle(h);
    }
#elif !defined(__EMSCRIPTEN__)
    if (mHandle != -1) {
        if (mLocked) flock(static_cast<int>(mHandle), LOCK_UN);
        close(static_cast<int>(mHandle));
    }
#endif
    mHandle = -1;
    mLocked = false;
}

bool replaceFile(const std::string& path, std::string_view contents)
{
    std::string tmp = tempName(path);
#ifdef _WIN32
    std::wstring wtmp = u8(tmp).wstring(), wpath = u8(path).wstring();
    HANDLE h = CreateFileW(wtmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    bool ok = true;
    for (size_t at = 0; ok && at < contents.size();) {
        DWORD n = 0;
        DWORD chunk = static_cast<DWORD>(std::min<size_t>(contents.size() - at, 1u << 30));
        ok = WriteFile(h, contents.data() + at, chunk, &n, nullptr) && n > 0;
        at += n;
    }
    ok = ok && FlushFileBuffers(h);
    CloseHandle(h);
    // A reader without FILE_SHARE_DELETE (or a virus scanner) can hold the
    // old file for a moment: try again briefly.
    for (int i = 0; ok && i < 20; i++) {
        if (MoveFileExW(wtmp.c_str(), wpath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return true;
        DWORD e = GetLastError();
        if (e != ERROR_ACCESS_DENIED && e != ERROR_SHARING_VIOLATION) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    DeleteFileW(wtmp.c_str());
    return false;
#elif !defined(__EMSCRIPTEN__)
    int fd = open(tmp.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd < 0) return false;
    bool ok = true;
    for (size_t at = 0; ok && at < contents.size();) {
        ssize_t n = write(fd, contents.data() + at, contents.size() - at);
        ok = n > 0;
        if (ok) at += static_cast<size_t>(n);
    }
    ok = ok && fsync(fd) == 0;
    ok = close(fd) == 0 && ok;
    if (ok && rename(tmp.c_str(), path.c_str()) == 0) {
        // the rename itself reaches the disk with the folder
        std::string dir = u8(path).parent_path().string();
        int d = open(dir.empty() ? "." : dir.c_str(), O_RDONLY | O_CLOEXEC);
        if (d >= 0) {
            fsync(d);
            close(d);
        }
        return true;
    }
    unlink(tmp.c_str());
    return false;
#else
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        if (!out.flush()) {
            out.close();
            std::remove(tmp.c_str());
            return false;
        }
    }
    if (std::rename(tmp.c_str(), path.c_str()) == 0) return true;
    std::remove(tmp.c_str());
    return false;
#endif
}

}
