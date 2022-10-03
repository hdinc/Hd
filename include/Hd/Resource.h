#pragma once

#include <utility>

namespace Hd {

namespace Resource {

    std::pair<const char*, long> getExeRes(const char* file);
    std::pair<const char*, long> getLibRes(const char* file);

}

}
