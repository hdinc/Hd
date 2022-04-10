#include <Hd.h>

int main(int argc, char** argv)
{
    Hd::Resource& res = Hd::Resource::getInstance(argv[0]);

    printf("%s\n", res.get_file("res/file1.txt"));
}
