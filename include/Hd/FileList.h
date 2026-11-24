// the entries of a folder as a table, the part a file manager and a file
// chooser share; and what they need from the system: places, typed paths,
// hidden files. Paths given and returned as text are UTF-8.

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Hd {

// a path as UTF-8 text and back (path::string() is the ANSI code page on
// Windows)
std::string toUtf8(const std::filesystem::path& p);
std::filesystem::path fromUtf8(std::string_view s);

// a < b by name, ignoring case, with runs of digits compared as numbers
// (file2 before file10)
bool naturalLess(const std::string& a, const std::string& b);

// hidden like the system's file managers show it: a name starting with a
// dot, or on Windows the hidden or system attribute
bool isHidden(const std::filesystem::path& p);

// a place to start from in a file dialog
struct Place {
    std::string name; // "Documents", "Local Disk (C:)"
    std::filesystem::path path;
    const char* icon = nullptr; // an Hd::Icon glyph
    bool drive = false; // a drive or the file system's root, not a folder
};
// home, the user's folders that exist (Desktop, Documents, Downloads,
// Pictures, ...), then the drives (Windows: letters with their labels) or
// the root ("/")
std::vector<Place> places();
// the folders of an XDG user-dirs.dirs file (~/.config/user-dirs.dirs):
// name ("Documents") and path, $HOME replaced by home; home itself and
// lines it can't read are left out
std::vector<std::pair<std::string, std::filesystem::path>> parseUserDirs(std::string_view text, const std::filesystem::path& home);

// a path typed into a path bar, as a path: "~" and "~/x" in home, a bare
// drive ("C:") as its root, relative names from dir, MSYS drive paths on
// Windows (/c/x), UNC paths (\\server\share) kept; normalised
std::filesystem::path typedPath(std::string_view text, const std::filesystem::path& dir, const std::filesystem::path& home);
// Tab in a path bar: text with the folder name it ends in completed as far
// as the folders there agree (case is ignored; the folder's own case is
// used); text when nothing matches
std::string completeFolder(std::string_view text, const std::filesystem::path& dir, const std::filesystem::path& home);

// lists dir in a table of name, size and date (where there's room), folders
// first. a click on a header sorts, names in natural order (file2 before
// file10). the folder is read on a worker thread when dir changes and once
// a second, so changes made elsewhere show up and slow drives don't stop
// the UI.
class FileList {
public:
    FileList();
    ~FileList();
    FileList(const FileList&) = delete;
    FileList& operator=(const FileList&) = delete;

    // the folder shown
    std::filesystem::path dir;
    // the highest folder it goes up to, none if empty
    std::filesystem::path root;
    // the selected entry (the last one clicked when several are), empty for
    // none
    std::filesystem::path selected;
    // all the selected entries, when multiple is on (otherwise selected only)
    std::vector<std::filesystem::path> selection;
    bool multiple = false;
    // which files are shown, all if not set. folders always are. call
    // refresh() when what it lets through changes.
    std::function<bool(const std::filesystem::path&)> show;
    // dot files and hidden ones
    bool showHidden = false;
    // home for "~" in typed paths, the user's home if empty
    std::filesystem::path home;

    // the folders from root to dir as buttons, a click goes there. a click
    // on its empty end (or Ctrl+L) edits the path as text: Tab completes a
    // folder name, Enter goes there, Escape stops.
    void pathBar();
    // the table, as high as height (0: the rest of the window). a double
    // click or Enter goes into a folder, or on a file returns true with that
    // file. keys while it is focused: type a name's start to select it,
    // up/down, Enter, Backspace or Alt+Up for the parent folder.
    bool table(std::filesystem::path& activated, float height = 0.0f);

    // reads the folder at the next frame, e.g. after changing it
    void refresh() { stale = true; }
    // dir, and selected if it's in there, scrolled to
    void go(const std::filesystem::path& folder, const std::filesystem::path& select = {});
    // the path bar is being typed in
    bool editingPath() const { return editing; }
    void editPath();

private:
    struct entry {
        std::filesystem::path path;
        std::string name;
        bool folder = false;
        bool hidden = false;
        uintmax_t size = 0;
        std::string date;
    };
    struct Listing; // a read in progress on the worker thread

    void startRead();
    bool takeRead(); // the finished read's entries, true if there was one
    void filter(); // all -> entries: show, showHidden
    void sort();
    void select(const std::filesystem::path& p, bool ctrl, bool shift);
    void keys(std::filesystem::path& into, std::filesystem::path& from, std::filesystem::path& activated, bool& chosen);
    std::filesystem::path userHome() const;

    std::vector<entry> all; // as read
    std::vector<entry> entries; // shown, sorted
    bool lastHidden = false; // showHidden when entries were filtered
    std::filesystem::path listed; // the folder entries are of
    std::string error;
    double readAt = -1.0;
    bool stale = true;
    std::shared_ptr<Listing> reading;
    // the table's sort, column and direction
    int sortColumn = 0;
    bool descending = false;
    bool scrollToSelected = false;
    std::filesystem::path anchor; // Shift+click selects from here
    std::string typed; // type-ahead
    double typedAt = 0;
    bool editing = false;
    bool focusEdit = false;
    std::string editText;
};

// a file size as "12 KB"
std::string formatSize(uintmax_t bytes);

}
