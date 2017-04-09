#include "utils.h"
#include <stdio.h>
#include <cstdarg>
#include <iostream>

void makeAssertion(const std::string &msg)
{
    std::cout << msg << std::endl;
}

void log(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vprintf(format, args);
    printf("\n");

    va_end(args);
}
