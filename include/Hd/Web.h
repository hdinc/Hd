#pragma once

// glue between a program and its page, in the web build (emscripten) only.
// the page's side is web/pre.js of the library, linked in with it.

#include <filesystem>
#include <vector>

namespace Hd::Web {

// the page asks before it's closed while there are unsaved changes
void setUnsaved(bool unsaved);

// opens the browser's file picker and copies the chosen files into dir,
// replacing ones with the same names. call it right after a click, browsers
// only open the picker then.
void upload(const std::filesystem::path& dir);
// files dropped on the page are copied into dir, the home folder at the
// start
void setDropDir(const std::filesystem::path& dir);
// the files uploaded or dropped since the last call, once they're written
std::vector<std::filesystem::path> takeUploaded();

// the browser saves the file to the user's computer
void download(const std::filesystem::path& file);

}
