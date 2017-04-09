#include <iostream>

#include "defines.h"
#include "utils.h"
#include "fileutils.h"
#include "matcher.h"
#include "simulator.h"
#include "verilog.h"

void printMatching(const Matching& match);

void printUsage()
{
    std::cout << "Usage: ./matcher <command> <arguments>" << std::endl;
    std::cout << "\t- stuck <in_file.v> <input_name> <value>" << std::endl;
    std::cout << "\t- inv <in_file.v> <input_name>" << std::endl;
    std::cout << "\t- cone <in_file.v> <output_name>" << std::endl;
    std::cout << "\t- copy <in_file.v>" << std::endl;
    std::cout << "\t- sim <in_file.v> <output_name> <num_of_iterations>" << std::endl;
}

int main(int argc, char * argv[])
{
    srand(time(NULL));

    if (argc < 2)
    {
        printf( "Wrong number of command-line arguments.\n" );
        printUsage();
        return FAIL;
    }

    std::string cmd(argv[1]);

    if (cmd == "stuck" && argc == 5)
    {
        char *in_file = argv[2];
        const std::string pi(argv[3]);
        int value = std::atoi(argv[4]);

        log("Stucking input %s with value %d", pi.c_str(), value);

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        cir->stuckInput(pi, value); //implicit cast to bool
        cir->print();

        delete cir;

        return OK;
    }
    else if (cmd == "inv_in" && argc == 4)
    {
        char *in_file = argv[2];
        const std::string pi(argv[3]);

        log("Inverting input %s", pi.c_str());

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        cir->invertInput(pi);
        cir->print();

        delete cir;

        return OK;
    }
    else if (cmd == "inv_out" && argc == 4)
    {
        char *in_file = argv[2];
        const std::string po(argv[3]);

        log("Inverting input %s", po.c_str());

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        cir->invertOutput(po);
        cir->print();

        delete cir;

        return OK;
    }
    else if (cmd == "cone" && argc == 4)
    {
        char *in_file = argv[2];
        const std::string po(argv[3]);

        log("Getting cone for output %s", po.c_str());

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        Circuit *cone = cir->getCone(po);
        cone->print();

        delete cir;
        delete cone;

        return OK;
    }
    else if (cmd == "copy" && argc == 3)
    {
        char *in_file = argv[2];

        log("Copying circuit %s", in_file);

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        Circuit *cir_copy = new Circuit(*cir);
        cir_copy->print();

        delete cir;
        delete cir_copy;

        return OK;
    }
    else if (cmd == "sim" && argc == 5)
    {
        char *in_file = argv[2];
        const std::string po(argv[3]);
        std::size_t sim_iterations = std::atol(argv[4]);

        log("Starting simulations for output %s (max = %u)", po.c_str(), sim_iterations);

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        Circuit *cone = cir->getCone(po);
        cone->print();

        Simulator sim(cone);
        auto input_properties = sim.simulate(sim_iterations);

        log("Input properties:");
        for (const auto &it : input_properties)
            log("%s: %s", it.first.c_str(), propSetToStr(it.second).c_str());

        delete cir;
        delete cone;

        return OK;
    }
    else
    {
        makeAssertion("Wrong command or number of arguments");
        printUsage();
        return FAIL;
    }

//    if (argc == 3)
//    {
//        char *in_file1 = argv[1];
//        char *in_file2 = argv[2];

//        Circuit *cir1 = parse_verilog(FileUtils::load_file(in_file1));
//        Circuit *cir2 = parse_verilog(FileUtils::load_file(in_file2));

//        IOSupport support = IOSupportCalculator(cir1).getOutputSupport();
//        for (auto& sup : support)
//            std::cout << "Output " << sup.first << " has " << sup.second.size() << " PIs in its support" << std::endl;

//        Matcher matcher(cir1, cir2);
//        for (size_t i = 0; i < 10; ++i)
//            printMatching(matcher.getResult());

//        return OK;
//    }

    return OK;
}

void printMatching(const Matching& match)
{
    std::cout << "MATCHING" << std::endl;

    std::cout << "SCORE=" << match.score << std::endl;

    std::cout << "STUCK_INPUTS_2=[ ";
    for (const auto &po : match.stuck_inputs2)
        std::cout << po << " ";
    std::cout << "]" << std::endl;

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



