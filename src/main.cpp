#include <iostream>

#include "defines.h"
#include "fileutils.h"
#include "matcher.h"
#include "verilog.h"

void printMatching(const Matching& match);

int main(int argc, char * argv[])
{
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
        printMatching(matcher.getResult());

        return OK;
    }

    return OK;
}

void printMatching(const Matching& match)
{
    std::cout << "MATCHING" << std::endl;

    std::cout << "SCORE=" << match.score << std::endl;

    for (const auto &cluster : match.input_clusters1)
    {
        std::cout << "INPUT_CLUSTER_1=[ ";
        for (const auto &i : cluster.second)
            std::cout << i << " ";
        std::cout << "]" << std::endl;
    }
    for (const auto &cluster : match.output_clusters1)
    {
        std::cout << "OUTPUT_CLUSTER_1=[ ";
        for (const auto &o : cluster.second)
            std::cout << o << " ";
        std::cout << "]" << std::endl;
    }

    for (const auto &cluster : match.input_clusters2)
    {
        std::cout << "INPUT_CLUSTER_2=[ ";
        for (const auto &i : cluster.second)
            std::cout << i << " ";
        std::cout << "]" << std::endl;
    }
    for (const auto &cluster : match.output_clusters2)
    {
        std::cout << "OUTPUT_CLUSTER_2=[ ";
        for (const auto &o : cluster.second)
            std::cout << o << " ";
        std::cout << "]" << std::endl;
    }

    std::cout << "END" << std::endl;
}



