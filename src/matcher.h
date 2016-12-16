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
std::string getPrefix(Signature sign);

struct CircuitData
{
    CircuitData();
    CircuitData(Circuit* cir);

    IOSet matched_inputs, unmatched_inputs;
    IOSet matched_outputs, unmatched_outputs;

    IOSupport output_support, input_support;

    IOCluster matched_output_clusters, unmatched_output_clusters, input_clusters;

    std::map<std::string, Signature> input_signatures, output_signatures;

    std::set<Signature> getUnmatchedSignatures();
    std::set<Signature> getMatchedSignatures();

    std::pair<IOSet, IOSet> match(const std::string &po);
    IOSet stickUnmatchedInputs(const std::string &po, size_t n);
private:
    void calculateInitClusters();
    IOSet getUnmatchedInputs(const std::string &po);
};

using IOMatching = std::vector<std::pair<IOSet, IOSet>>;

struct Matching
{
    IOMatching input_matching, output_matching;
    IOSet stuck_inputs2;
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
    std::pair<Signature, Signature> getMinDiffSignatures();
    void match(const std::string &po1, const std::string &po2);
};

#endif // __MATCHING_DATA_H__
