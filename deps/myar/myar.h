#include <string>
#include <vector>
#include <utility>

namespace myar {

struct myar_file {
    std::string name;
    std::vector<char> data;
};

struct myar_header {
    unsigned long name_offset;
    unsigned long data_offset;
    unsigned long data_size;
};

class reader {

private:
    const char* buf;
    long size;

    long num_file;

    std::vector<myar_header> header;

    bool del_buf = false;
    bool loaded = false;

public:
    ~reader();

    void load(const char* buf, long size);
    void load(std::string filename);
    std::pair<const char*, long> get_file(std::string filename);
    void list_files();
    bool is_loaded() { return loaded; }
};

class archiver {
private:
    std::vector<myar_file> files;

public:
    archiver();
    void add_file(std::string filename, std::vector<char> data);
    void add_dir(std::string dirname);
    void write(std::string filename);

    std::vector<char> read_file(std::string filename);
};

}
