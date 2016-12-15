#ifndef __MATCHER_H__
#define __MATCHER_H__

#include <set>
#include <map>
#include <string>

#include "circuit.h"

struct CircuitData
{
    std::set<std::string> matched_inputs, unmatched_inputs;
    std::set<std::string> matched_outputs, unmatched_outputs;

    std::multimap<std::string, std::string> input_support, output_support;

    std::multimap<std::string, std::string> input_clusters, output_clusters;

    std::map<std::string, std::string> input_signatures, output_signatures;
};


class Matcher
{
public:
    Matcher(Circuit *cir1, Circuit *cir2);
private:
    Circuit *cir1, *cir2;
    CircuitData data1, data2;

    void reset();
};

#endif // __MATCHING_DATA_H__
