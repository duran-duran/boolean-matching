#include <iostream>

#include "defines.h"
#include "fileutils.h"
#include "matcher.h"
#include "verilog.h"

void printMatching(const Matching& match);

int main(int argc, char * argv[])
{
    srand(time(NULL));

    if (argc < 3)
    {
        printf( "Wrong number of command-line arguments.\n" );
        return FAIL;
    }

    if (argc == 3)
    {
        char *in_file1 = argv[1];
        char *in_file2 = argv[2];

        Circuit *cir1 = parse_verilog(FileUtils::load_file(in_file1));
        Circuit *cir2 = parse_verilog(FileUtils::load_file(in_file2));

//        IOSupport support = IOSupportCalculator(cir1).getOutputSupport();
//        for (auto& sup : support)
//            std::cout << "Output " << sup.first << " has " << sup.second.size() << " PIs in its support" << std::endl;

        Matcher matcher(cir1, cir2);
        for (size_t i = 0; i < 100; ++i)
            printMatching(matcher.getResult());

        return OK;
    }

    return OK;
}

void printMatching(const Matching& match)
{
    std::cout << "MATCHING" << std::endl;

    std::cout << "SCORE=" << match.score << std::endl;

    for (const auto &i_matching : match.input_matching)
    {
        std::cout << "INPUT_GROUP=[ [ ";

        for (const auto &pi : i_matching.first)
            std::cout << pi << " ";

        std::cout << "] [ ";

        for (const auto &pi : i_matching.second)
            std::cout << pi << " ";

        std::cout << "] ]" << std::endl;
    }

    for (const auto &o_matching : match.output_matching)
    {
        std::cout << "OUTPUT_GROUP=[ [ ";

        for (const auto &po : o_matching.first)
            std::cout << po << " ";

        std::cout << "] [ ";

        for (const auto &po : o_matching.second)
            std::cout << po << " ";

        std::cout << "] ]" << std::endl;
    }

    std::cout << "END" << std::endl;
}



