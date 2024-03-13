#include <Hd/Resource.h>
#include <myar.h>

extern const char _binary_exeres_myar_start[];
extern const char _binary_exeres_myar_end[];

extern const char _binary_libres_myar_start[];
extern const char _binary_libres_myar_end[];

namespace Hd {

namespace Resource {

    static myar::reader libreader;
    static myar::reader exereader;

    std::pair<const char*, long> getLibRes(const char* file)
    {
        if (!libreader.is_loaded()) {
            libreader.load(_binary_libres_myar_start, _binary_libres_myar_end - _binary_libres_myar_start);
        }

        return libreader.get_file(file);
    }

    std::pair<const char*, long> getExeRes(const char* file)
    {
        if (!exereader.is_loaded()) {
            exereader.load(_binary_exeres_myar_start, _binary_exeres_myar_end - _binary_exeres_myar_start);
        }

        return exereader.get_file(file);
    }

}

}
