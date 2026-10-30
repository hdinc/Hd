#include <Hd/Web.h>

#include <emscripten.h>

#include <cstdlib>

namespace Hd::Web {

namespace {

EM_JS_DEPS(hd_web, "$UTF8ToString,$stringToNewUTF8");

EM_JS(void, jsSetUnsaved, (int unsaved), {
    Module.hdUnsaved = !!unsaved;
});

EM_JS(void, jsUpload, (const char* dir), {
    Module.hdUpload(UTF8ToString(dir));
});

EM_JS(void, jsSetDropDir, (const char* dir), {
    Module.hdDropDir = UTF8ToString(dir);
});

// the next uploaded file, null if there's none. freed by the caller.
EM_JS(char*, jsTakeUploaded, (), {
    var f = Module.hdUploaded.shift();
    return f === undefined ? 0 : stringToNewUTF8(f);
});

EM_JS(void, jsDownload, (const char* file), {
    Module.hdDownload(UTF8ToString(file));
});

}

void setUnsaved(bool unsaved)
{
    static int last = -1;
    if (last == (int)unsaved) return;
    last = unsaved;
    jsSetUnsaved(unsaved);
}

void upload(const std::filesystem::path& dir)
{
    jsUpload(dir.c_str());
}

void setDropDir(const std::filesystem::path& dir)
{
    static std::filesystem::path last;
    if (dir == last) return;
    last = dir;
    jsSetDropDir(dir.c_str());
}

std::vector<std::filesystem::path> takeUploaded()
{
    std::vector<std::filesystem::path> r;
    while (auto s = jsTakeUploaded()) {
        r.push_back(s);
        std::free(s);
    }
    return r;
}

void download(const std::filesystem::path& file)
{
    jsDownload(file.c_str());
}

}
