#include <iostream>
#include <chrono>

#include "defines.h"
#include "utils.h"
#include "fileutils.h"
#include "matcher.h"
#include "simulator.h"
#include "verilog.h"
#include "checker.h"

//void printMatching(const Matching& match);
void printPartition(const POPartition &partition);
void printSymPartition(const SymmetryPartition &partition);

void printUsage()
{
    std::cout << "Usage: ./matcher <command> <arguments>" << std::endl;
    std::cout << "\t- stuck <in_file.v> <input_name> <value>" << std::endl;
    std::cout << "\t- inv_in <in_file.v> <input_name>" << std::endl;
    std::cout << "\t- inv_out <in_file.v> <output_name>" << std::endl;
    std::cout << "\t- miter <in_file.v> <output_name1> <output_name2> <function>" << std::endl;
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
    else if (cmd == "miter" && argc == 6)
    {
        char *in_file = argv[2];
        const std::string po1(argv[3]);
        const std::string po2(argv[4]);
        const std::string func(argv[5]);

        log("Constructing miter circuit from cones for outputs %s and %s", po1.c_str(), po2.c_str());

        Circuit *cir = parse_verilog(FileUtils::load_file(in_file));
        cir->print();

        Circuit *cone1 = cir->getCone(po1);
        cone1->print();

        Circuit *cone2 = cir->getCone(po2);
        cone2->print();

        const std::map<std::string, Function> func_map =
        {
            {"AND", FUNCTION_AND},
            {"OR" , FUNCTION_OR },
            {"XOR", FUNCTION_XOR},
            //..and so on
        };

        Circuit *miter = Circuit::getMiter(cone1, cone2, func_map.at(func));
        miter->print();

        checkMiter(miter);

        delete cir;
        delete cone1;
        delete cone2;
        delete miter;

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
//        auto input_symmetries = sim.simulateSym(sim_iterations);
        auto sym_partition = sim.simulateSym(sim_iterations);
        auto sv_symmetries = sim.simulateSVSym(sim_iterations);

        log("Input properties:");
        for (const auto &it : input_properties)
            log("%s: %s", it.first.c_str(), propSetToStr(it.second).c_str());

        log("Input symmetries:");
        printSymPartition(sym_partition);
//        for (const auto &it : input_symmetries)
//            log("%s, %s: %s", it.first.first.c_str(), it.first.second.c_str(), symSetToStr(it.second).c_str());

        log("SV symmetries:");
        for (const auto &it : sv_symmetries)
            log("%s: %s", it.first.c_str(), svSymSetToStr(it.second).c_str());

        delete cir;
        delete cone;

        return OK;
    }
    else if (cmd == "split" && argc == 4)
    {
        const char *in_file1 = argv[2],
                   *in_file2 = argv[3];

        Circuit *cir1 = parse_verilog(FileUtils::load_file(in_file1)),
                *cir2 = parse_verilog(FileUtils::load_file(in_file2));

        Matcher matcher(cir1, cir2);

        constexpr std::size_t max_it = 1000;

        double init_matchings_cnt = permCount(cir1->getInputs().size(), cir2->getInputs().size()) *
                                                combCount(cir1->getOutputs().size(), cir2->getOutputs().size());
        log("output matches: %e", combCount(cir1->getOutputs().size(), cir2->getOutputs().size()));
        log("input matches: %e", permCount(cir1->getInputs().size(), cir2->getInputs().size()));
        log("Initial possible matchings: %e", init_matchings_cnt);

        std::chrono::time_point<std::chrono::system_clock> start, end;
        std::chrono::milliseconds elapsed;
        log("Splitting by support...");
        start = std::chrono::system_clock::now();
        matcher.splitBySupport();
        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        log("Elapsed %dms", elapsed.count());
        log("Possible matchings: %e", matcher.calculatePossibleMatchings());

        log("Splitting by unateness...");
        start = std::chrono::system_clock::now();
        matcher.splitByUnateness();
        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        log("Elapsed %dms", elapsed.count());
        log("Possible matchings: %e", matcher.calculatePossibleMatchings());

        log("Splitting by symmetry...");
        start = std::chrono::system_clock::now();
        matcher.splitBySymmetry();
        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        log("Elapsed %dms", elapsed.count());
        log("Possible matchings: %e", matcher.calculatePossibleMatchings());

        log("Splitting by simulation type 1 (max_it = %u)...", max_it);
        start = std::chrono::system_clock::now();
        matcher.splitBySimType1(max_it);
        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        log("Elapsed %dms", elapsed.count());
        log("Possible matchings: %e", matcher.calculatePossibleMatchings());

        log("Splitting by simulation type 2 (max_it = %u)...", max_it);
        start = std::chrono::system_clock::now();
        matcher.splitBySimType2(max_it);
        end = std::chrono::system_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        log("Elapsed %dms", elapsed.count());
        log("Possible matchings: %e", matcher.calculatePossibleMatchings());

//        auto partitions = matcher.getPOPartitions();
//        printPartition(partitions.first);
//        printPartition(partitions.second);
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

//void printMatching(const Matching& match)
//{
//    std::cout << "MATCHING" << std::endl;

//    std::cout << "SCORE=" << match.score << std::endl;

//    std::cout << "STUCK_INPUTS_2=[ ";
//    for (const auto &po : match.stuck_inputs2)
//        std::cout << po << " ";
//    std::cout << "]" << std::endl;

//    for (const auto &i_matching : match.input_matching)
//    {
//        std::cout << "INPUT_GROUP=[ [ ";

//        for (const auto &pi : i_matching.first)
//            std::cout << pi << " ";

//        std::cout << "] [ ";

//        for (const auto &pi : i_matching.second)
//            std::cout << pi << " ";

//        std::cout << "] ]" << std::endl;
//    }

//    for (const auto &o_matching : match.output_matching)
//    {
//        std::cout << "OUTPUT_GROUP=[ [ ";

//        for (const auto &po : o_matching.first)
//            std::cout << po << " ";

//        std::cout << "] [ ";

//        for (const auto &po : o_matching.second)
//            std::cout << po << " ";

//        std::cout << "] ]" << std::endl;
//    }

//    std::cout << "END" << std::endl;
//}

std::string IOSetToStr(const IOSet &io_set)
{
    std::string result;
    for (const auto& io : io_set)
        result += io + " ";
    return result.substr(0, result.size() - 1);
}

std::string PISignMaskToStr(const PISignMask &input_signatures)
{
    std::string result;
    for (const auto &sign : input_signatures)
    {
        result += "{size: " + std::to_string(sign.first) + ", ";
        result += "unateness: " + propToStr(sign.second.unat) + ", ";
        result += "symmetry: " + symToStr(sign.second.sym);
        if (!sign.second.simType1.empty())
        {
            result += ", type1 sim results: [";
            for (bool val : sign.second.simType1)
                result += (val ? "1" : "0");
            result += "]";
        }
        if (!sign.second.simType2.empty())
        {
            result += ", type2 sim results: [";
            for (int val : sign.second.simType2)
                result += std::to_string(val) + ",";
            result = result.substr(0, result.size() - 1) + "]";
        }
        result += "}";
    }
    return result;
}

void printSymPartition(const SymmetryPartition &partition)
{
    for (const auto &cluster : partition)
        log("Symmetry: %s, inputs: %s", symToStr(cluster.first).c_str(), IOSetToStr(cluster.second).c_str());
}

void printPartition(const POPartition &partition)
{
    std::size_t i = 0;
    for (const auto &cluster : partition)
    {
        log("Cluster %u:", i);
        log("Outputs: %s", IOSetToStr(cluster.second).c_str());
        log("Support size: %u", cluster.first.support_size);
        log("Input partition: %s", PISignMaskToStr(cluster.first.input_signatures).c_str());
        log("---------");
        ++i;
    }
}

