#pragma once

#include <cstdint>
#include <string>
#include <string_view>

// Safe writes to files that several processes share, such as settings.
// Paths are UTF-8.
namespace Hd {

// An exclusive lock between processes, held on a lock file (which it
// creates). The operating system releases it when the process exits, also
// if it crashes, so a stale lock never blocks. Windows uses LockFileEx,
// elsewhere flock. In the browser there are no other processes: it is
// always taken.
class FileLock {
public:
    FileLock() = default;
    // waits up to `timeout` seconds for the lock; see locked()
    explicit FileLock(const std::string& path, double timeout = 5);
    ~FileLock() { unlock(); }
    FileLock(FileLock&& o) noexcept;
    FileLock& operator=(FileLock&& o) noexcept;
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;

    bool locked() const { return mLocked; }
    void unlock();

private:
    intptr_t mHandle = -1; // a Windows HANDLE or a file descriptor
    bool mLocked = false;
};

// Replaces the file's contents in one step: they are written to a temporary
// file in the same folder, flushed to disk, then moved over `path`. Readers
// see the old file or the new one, never part of either. On failure the old
// file is left as it was and the temporary file is removed. The folder must
// exist.
bool replaceFile(const std::string& path, std::string_view contents);

}
