#include "utils.h"
#include <stdio.h>
#include <cstdarg>
#include <iostream>
#include <cmath>

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

double combCount(std::size_t a, std::size_t b)
{
    auto fact = [](std::size_t n)
    {
        std::size_t result = 1;
        for (std::size_t i = 1; i < n; ++i)
            result *= i;
        return result;
    };

    return (a < b) ? fact(b) / fact(b - a) : fact(a) / fact(a - b);
}

double permCount(std::size_t a, std::size_t b)
{
    return std::pow(a, b)/*std::pow(std::min(a, b), std::max(a, b))*/;
}
