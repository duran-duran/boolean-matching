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

Signature updateSignature(const Signature &old_sign, char cluster_id, size_t n);

struct CircuitData
{
    CircuitData();
    CircuitData(Circuit* cir);

    IOSet matched_inputs, unmatched_inputs;
    IOSet matched_outputs, unmatched_outputs;

    IOSupport output_support, input_support;

    IOCluster output_clusters, input_clusters;

    std::map<std::string, Signature> input_signatures, output_signatures;

    std::set<Signature> getUnmatchedSignatures();
    std::set<Signature> getMatchedSignatures();

    IOSet match(const std::string &po);
private:
    void calculateInitClusters();

    IOSet getUnmatchedInputs(const std::string &po);
};

using IOMatching = std::vector<std::pair<IOSet, IOSet>>;

struct Matching
{
    IOMatching input_matching, output_matching;
    size_t score;
    //actions

    Matching() : score(0) {}
};

class Matcher
{
public:
    Matcher(Circuit *cir1, Circuit *cir2);

    Matching getResult();
private:
    Circuit *cir1, *cir2;
    CircuitData data1, data2;

    Matching cur_match;

    std::set<Signature> getCommonUnmatchedSignatures();
    void match(const std::string &po1, const std::string &po2);
};

#endif // __MATCHING_DATA_H__
