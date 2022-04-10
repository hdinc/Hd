#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <string>
#include "myar.h"

void usage()
{
    printf("Usage: myar <archive|extract> <dir|file> [-o <name>]\n");
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        usage();
        exit(1);
    }

    if (!strcmp(argv[1], "archive")) {

        std::string dir = argv[2];
        std::string name;

        if (argc != 5) {
            printf("wrong argument\n");
            usage();
            exit(1);
        }
        if (strcmp(argv[3], "-o")) {
            printf("wrong argument\n");
            usage();
            exit(1);
        } else {

            name = argv[4];
        }

        // FILE* fp = fopen(name.c_str(), "w");

        // if (!fp) {
        //     printf("cannot open %s\n", name.c_str());
        //     exit(1);
        // }

        myar::archiver archiver;

        archiver.add_dir(dir);
        archiver.write(name);

    } else if (!strcmp(argv[1], "extract")) {
        if (argc != 3) {
            printf("wrong argument\n");
            usage();
            exit(1);
        }

        std::string name = argv[2];

        FILE* fp = fopen(name.c_str(), "rb");

        if (!fp) {
            printf("cannot open file: %s\n", name.c_str());
            exit(1);
        }

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* buf = new char[size];

        fread(buf, size, 1, fp);

        myar::reader reader;
        reader.load(buf, size);

        delete[] buf;

    } else {

        usage();
        exit(1);
    }
}
