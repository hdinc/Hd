#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include "myar.h"

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
        printf("this is not a MYAR archive\n");
        exit(1);
    }

    if (memcmp(buf, &magic, 4)) {
        printf("this is not a MYAR archive\n");
        exit(1);
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

const char* reader::get_file(std::string filename)
{
    if (!loaded)
        return 0;

    for (int i = 0; i < header.size(); i++) {
        if (!strcmp(filename.c_str(), &buf[header[i].name_offset])) {
            return &buf[header[i].data_offset];
        }
    }
    return 0;
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
    DIR* dp = opendir(dirname.c_str());

    if (!dp) {
        printf("cannot open dir: %s\n", dirname.c_str());
        printf("%s\n", strerror(errno));
        exit(1);
    }

    dirent* dirp;

    if (!dp) {
        printf("cannot open dir\n");
        exit(1);
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
            continue;
        if (dirp->d_type == DT_DIR) {
            add_dir(dirname + "/" + dirp->d_name);
        } else if (dirp->d_type == DT_REG) {

            // read data
            std::vector<char> data = read_file(dirname + "/" + dirp->d_name);

            // add null for string files
            data.push_back(0);

            files.push_back({ dirname + "/" + dirp->d_name, data });
            printf("%s\n", files[files.size() - 1].name.data());
        }
    }
}

void archiver::write(std::string file)
{
    FILE* fp;

    // check if file exist
    fp = fopen(file.c_str(), "r");

    if (fp) {
        printf("file %s exist\n", file.c_str());
        exit(1);
    }

    // reopen file in write mode
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
        header.push_back({ 0, 0, files[i].data.size() });
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
