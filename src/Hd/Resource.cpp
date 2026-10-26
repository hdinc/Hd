#include <Hd/Resource.h>
#include <myar.h>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
// arrays made by add_resource_dir, wasm-ld can't link the archives as they are
extern "C" const char exeres_myar[];
extern "C" const long exeres_myar_size;
extern "C" const char libres_myar[];
extern "C" const long libres_myar_size;
#define HD_EXERES exeres_myar, exeres_myar_size
#define HD_LIBRES libres_myar, libres_myar_size
#else
extern char _binary_exeres_myar_start;
extern char _binary_exeres_myar_end;

extern char _binary_libres_myar_start;
extern char _binary_libres_myar_end;
#define HD_EXERES &_binary_exeres_myar_start, &_binary_exeres_myar_end - &_binary_exeres_myar_start
#define HD_LIBRES &_binary_libres_myar_start, &_binary_libres_myar_end - &_binary_libres_myar_start
#endif

namespace Hd {

namespace Resource {

    static myar::reader libreader;
    static myar::reader exereader;

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
