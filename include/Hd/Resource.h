#pragma once

#include <elfio/elfio.hpp>
#include <myar.h>

namespace Hd {

class Resource {

public:
    static Resource& getInstance(const char* elf_file);
    Resource(Resource const&) = delete;
    void operator=(Resource const&) = delete;

    const char* get_file(std::string filename);
    long get_file_size(std::string filename);

    Resource(const char* elf_file);
    ~Resource();

private:
    ELFIO::elfio elf_reader;
    myar::reader myar_reader;

    const char* myar_data = 0;
    long myar_data_size = 0;
};
}
