#include <Hd/Resource.h>

namespace Hd {

Resource& Resource::getInstance(const char* elf_file)
{
    static Resource Resource(elf_file);
    return Resource;
}

Resource::Resource(const char* elf_file)
{
    elf_reader.load(elf_file);

    for (int i = 0; i < elf_reader.sections.size(); i++) {
        if (elf_reader.sections[i]->get_name() == "res.myar") {
            myar_data = elf_reader.sections[i]->get_data();
            myar_data_size = elf_reader.sections[i]->get_size();
        }
    }

    myar_reader.load(myar_data, myar_data_size);
}

Resource::~Resource()
{
}

const char* Resource::get_file(std::string filename)
{
    return myar_reader.get_file(filename);
}

}
