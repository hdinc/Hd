#include <Hd/Resource.h>
#include <hdres.h>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
// arrays made by add_resource_dir, wasm-ld can't link the archives as they are
extern "C" const char exeres_hdres[];
extern "C" const long exeres_hdres_size;
extern "C" const char libres_hdres[];
extern "C" const long libres_hdres_size;
#define HD_EXERES exeres_hdres, exeres_hdres_size
#define HD_LIBRES libres_hdres, libres_hdres_size
#else
extern char _binary_exeres_hdres_start;
extern char _binary_exeres_hdres_end;

extern char _binary_libres_hdres_start;
extern char _binary_libres_hdres_end;
#define HD_EXERES &_binary_exeres_hdres_start, &_binary_exeres_hdres_end - &_binary_exeres_hdres_start
#define HD_LIBRES &_binary_libres_hdres_start, &_binary_libres_hdres_end - &_binary_libres_hdres_start
#endif

namespace Hd {

namespace Resource {

    static hdres::reader libreader;
    static hdres::reader exereader;

    std::pair<const char*, long> getLibRes(const char* file)
    {
        if (!libreader.is_loaded()) {
            libreader.load(HD_LIBRES);
        }

        auto r = libreader.get_file(file);
        if (r.first) {
            return r;
        } else throw std::runtime_error("getLibRes: File not found : " + std::string(file));
    }

    std::pair<const char*, long> getExeRes(const char* file)
    {
        if (!exereader.is_loaded()) {
            exereader.load(HD_EXERES);
        }

        auto r = exereader.get_file(file);
        if (r.first) {
            return r;
        } else throw std::runtime_error("getExeRes: File not found : " + std::string(file));
    }

}

}
