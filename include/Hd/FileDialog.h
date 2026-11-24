// choosing files: a modal dialog over Hd::FileList

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Hd {

// filters of browseButton and openDialog: comma separated, each a list of
// extensions with a description, "Description{.a,.b}", one extension ".a",
// or ".*" for all files. the first one is chosen at the start.
//
// a "..." button that opens a file dialog at path. true when a file was
// chosen, path is then that file: relative to base if it's inside base,
// absolute otherwise. call it every frame while the dialog may be open, it
// draws the dialog too.
bool browseButton(const char* id, std::string& path, const std::filesystem::path& base, const char* filters, const char* title = "Choose a file");

// a dialog to open or save a file, started by openDialog and drawn by
// dialogResult every frame. dialogResult is true once a file was chosen,
// file is then its absolute path. saving asks before overwriting and adds
// the filter's extension to a name without one.
//
// the dialog has the places (home, the user's folders, drives) and recent
// folders on its left, a path bar that can be typed in (Tab completes), the
// folder's entries (read on a worker thread; keys: type a name, arrows,
// Enter, Backspace), "Show hidden", and "New folder" when saving or choosing
// a folder.
void openDialog(const char* id, const char* title, const char* filters, const std::filesystem::path& start, bool save = false);
bool dialogResult(const char* id, std::filesystem::path& file);
// the same for choosing a folder: dialogResult gives the folder (the one
// selected, or the one shown)
void openFolderDialog(const char* id, const char* title, const std::filesystem::path& start);
// the same for opening several files (Ctrl+click, Shift+click); the result
// with dialogResults
void openFilesDialog(const char* id, const char* title, const char* filters, const std::filesystem::path& start);
bool dialogResults(const char* id, std::vector<std::filesystem::path>& files);

// the folders files were last chosen in, newest first, shown under the
// places; a program may keep them between runs
void setRecentFolders(std::vector<std::filesystem::path> folders);
const std::vector<std::filesystem::path>& recentFolders();

// the dialogs don't go above root (none if empty), and their Home button
// goes there, e.g. to keep them in a workspace. name is the button's tooltip.
void setDialogRoot(const std::filesystem::path& root, const std::string& name = "The top folder");
// an upload button in the dialogs, upload(dir) copies files from the user's
// computer into dir, e.g. Web::upload in the browser.
void setDialogUpload(std::function<void(const std::filesystem::path&)> upload);
// the folder of the open dialog, empty if none is open
std::filesystem::path dialogFolder();
// selects a file that was just uploaded in the open dialog. false if none
// is open.
bool dialogShowFile(const std::filesystem::path& file);

}
