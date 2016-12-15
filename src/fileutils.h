#include <string>

namespace FileUtils
{
    std::string load_file(const std::string &filename);
    std::string load_file();
    int file_copy(char * in, char * out);
    std::string md5_sum(const std::string &filename);
}
