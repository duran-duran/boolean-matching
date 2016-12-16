#ifndef __MATCHER_H__
#define __MATCHER_H__

#include <set>
#include <map>
#include <string>

#include "circuit.h"

using IOSet = std::set<std::string>;
using IOSupport = std::map<std::string, IOSet>;

class IOSupportCalculator
{
public:
    IOSupportCalculator(Circuit *cir);

    IOSupport getOutputSupport();
    static IOSupport swapSupport(const IOSupport& support);
private:
    Circuit *cir;
    IOSupport supportCache;

    IOSet getSupport(const std::string &p);
};

using Signature = std::string;
using IOCluster = std::map<Signature, IOSet>;

struct CircuitData
{
    CircuitData();
    CircuitData(Circuit* cir);

    IOSet matched_inputs, unmatched_inputs;
    IOSet matched_outputs, unmatched_outputs;

    IOSupport output_support, input_support;

    IOCluster output_clusters, input_clusters;

    std::map<std::string, Signature> input_signatures, output_signatures;
private:
    void calculateInitClusters();
};

struct Matching
{
    IOCluster output_clusters1, input_clusters1,
              output_clusters2, input_clusters2;
    size_t score;
    //actions

    Matching(const IOCluster &o_clusters1, const IOCluster &i_clusters1,
             const IOCluster &o_clusters2, const IOCluster &i_clusters2,
             size_t score) :
        output_clusters1(o_clusters1), input_clusters1(i_clusters1),
        output_clusters2(o_clusters2), input_clusters2(i_clusters2),
        score(score)
    {}
};


class Matcher
{
public:
    Matcher(Circuit *cir1, Circuit *cir2);

    Matching getResult();
private:
    Circuit *cir1, *cir2;
    CircuitData data1, data2;
};

#endif // __MATCHING_DATA_H__
