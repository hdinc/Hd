#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include <filesystem>
#include "myar.h"

// TODO: write files first then header

namespace myar {

char magic[4] = { 'M', 'Y', 'A', 'R' };

reader::~reader()
{
    if (del_buf)
        delete[] buf;
}

void reader::load(const char* buf, long size)
{
    this->size = size;
    this->buf = buf;

    loaded = true;

    if (size < sizeof(magic) + sizeof(long)) {
        throw std::runtime_error("this is not a MYAR archive");
    }

    if (memcmp(buf, &magic, 4)) {
        throw std::runtime_error("this is not a MYAR archive");
    }

    num_file = *(long*)&buf[4];

    long offset = sizeof(magic) + sizeof(long);

    for (int i = 0; i < num_file; i++) {
        header.push_back(*(myar_header*)&buf[offset]);
        offset += sizeof(myar_header);
    }
}

void reader::load(std::string filename)
{
    FILE* fp = fopen(filename.c_str(), "rb");

    if (!fp) {
        printf("cannot open file: %s\n", filename.c_str());
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = new char[size];
    del_buf = true;

    fread(buf, size, 1, fp);

    fclose(fp);

    load(buf, size);
}

std::pair<const char*, long> reader::get_file(std::string filename)
{
    if (!loaded)
        return { 0, 0 };

    using namespace std::filesystem;

    path f1 = filename;

    for (int i = 0; i < header.size(); i++) {
        path f2 = std::string(&buf[header[i].name_offset]);
        if (f1 == f2) {
            return { &buf[header[i].data_offset], header[i].data_size };
        }
    }
    return { 0, 0 };
}

void reader::list_files()
{
    if (!loaded)
        return;

    for (int i = 0; i < num_file; i++) {
        printf("%s\n", &buf[header[i].name_offset]);
    }
}

archiver::archiver() { }

void archiver::add_dir(std::string dirname)
{
    using namespace std::filesystem;

    if (!(exists(dirname) && is_directory(dirname))) {
        printf("%s is not a directory\n", dirname.c_str());
        exit(1);
    }

    path base = path(dirname + "/");
    path basename = base.parent_path().filename();

    for (const auto& e : recursive_directory_iterator(dirname)) {
        if (e.is_regular_file()) {
            // read data
            std::vector<char> data = read_file(e.path().string());

            // add null for text files
            data.push_back(0);

            path p = basename / relative(e.path(), dirname);

            files.push_back({ p.string(), data });
            printf("%s\n", files.back().name.c_str());
        }
    }
}

void archiver::write(std::string file)
{
    FILE* fp;

    fp = fopen(file.c_str(), "wb");

    if (!fp) {
        printf("create_arhcive: cannot open file %s\n", file.c_str());
        exit(1);
    }

    std::vector<myar_header> header;
    std::vector<std::string> name;
    std::vector<std::vector<char>> data;

    unsigned long name_size = 0;

    for (int i = 0; i < files.size(); i++) {
        header.push_back({ 0, 0, static_cast<unsigned long>(files[i].data.size()) });
        name.push_back(files[i].name);
        data.push_back(files[i].data);

        name_size += files[i].name.size() + 1;
    }

    for (int i = 0; i < files.size(); i++) {
        header[i].data_size = data[i].size();

        unsigned long name_offset = 0;
        unsigned long data_offset = 0;

        for (int j = 0; j < i; j++) {
            name_offset += name[j].size() + 1;
            data_offset += data[j].size();
        }

        header[i].name_offset = sizeof(magic) + sizeof(long) + sizeof(myar_header) * header.size() + name_offset;
        header[i].data_offset = sizeof(magic) + sizeof(long) + sizeof(myar_header) * header.size() + name_size + data_offset;
    }

    unsigned long numbfile = files.size();

    fwrite(magic, sizeof(magic), 1, fp);

    fwrite(&numbfile, sizeof(long), 1, fp);

    fwrite(header.data(), sizeof(myar_header), header.size(), fp);

    for (int i = 0; i < name.size(); i++) {
        fwrite(name[i].data(), name[i].size() + 1, 1, fp);
    }

    for (int i = 0; i < data.size(); i++) {
        fwrite(data[i].data(), data[i].size(), 1, fp);
    }

    fclose(fp);
}

std::vector<char> archiver::read_file(std::string filename)
{
    std::vector<char> file;

    FILE* fp = fopen(filename.c_str(), "rb");

    if (!fp) {
        printf("cannot open file: %s\n", filename.c_str());
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = new char[size];
    fread(buf, size, 1, fp);

    for (int i = 0; i < size; i++) {
        file.push_back(buf[i]);
    }

    return file;
}

}
