#include "fileutils.h"
#include "defines.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <openssl/md5.h>

namespace FileUtils
{

std::string load_file(const std::string &filename) {
    std::ifstream f(filename);
    std::string result;
    char c;
    while (f.get(c)) {
        result += c;
    }
    return result;
}

std::string load_file() {
    std::string result;
    char c;
    while (std::cin.get(c)) {
        result += c;
    }
    return result;
}

int file_copy(char * in, char * out)
{
  FILE * src;
  FILE * dst;
  char ch;

  // open files
  src = fopen(in, "r");
  if (src == NULL)
  {
    printf("Cannot open file %s \n", in);
    return FAIL;
  }

  dst = fopen(out, "w");
  if (dst == NULL)
  {
    fclose(src);
    printf("Cannot open file %s \n", out);
    return FAIL;
  }

  // copy files
  while ((ch = fgetc(src)) != EOF ) fputc(ch, dst);

  fclose(src);
  fclose(dst);
  return OK;
}

std::string md5_sum(const std::string &filename) {
    std::string data = load_file(filename);
    static unsigned char result[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char *>(data.c_str()), data.length(), result);
    std::stringstream hash;
    for (unsigned i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        hash << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(result[i]);
    }
    return hash.str();
}

}
