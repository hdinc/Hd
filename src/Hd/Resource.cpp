#include <Hd/Resource.h>
#include <myar.h>

extern const char _binary_exe_res_myar_start[];
extern const char _binary_exe_res_myar_end[];

extern const char _binary_lib_res_myar_start[];
extern const char _binary_lib_res_myar_end[];

namespace Hd {

namespace Resource {

    static myar::reader exereader;
    static myar::reader libreader;

    static void loadexereader()
    {
        if (!exereader.is_loaded()) {
            exereader.load(_binary_exe_res_myar_start, _binary_exe_res_myar_end - _binary_exe_res_myar_start);
        }
    }

    static void loadlibreader()
    {
        if (!libreader.is_loaded()) {
            libreader.load(_binary_lib_res_myar_start, _binary_lib_res_myar_end - _binary_lib_res_myar_start);
        }
    }

    std::pair<const char*, long> getExeRes(const char* file)
    {
        loadexereader();

        return exereader.get_file(file);
    }

    std::pair<const char*, long> getLibRes(const char* file)
    {
        loadlibreader();

        return libreader.get_file(file);
    }

}
}
