#pragma once

#include <map>
#include "circuit.h"
#include "signdata.h"

using Matching = std::map<std::string, std::string>;
using Match = Matching::value_type;

using MultiMatching = std::vector<std::pair<IOSet, IOSet>>;

struct ValidationResult
{
    Matching valid_matches, invalid_matches;
};

struct CheckResult
{
    bool valid;
    MultiMatching inputMatching;
};

class Validator
{
public:
    static ValidationResult validateMatching(Circuit *cir1, Circuit *cir2,
                                             const SignData &signs1, const SignData &signs2,
                                             const Matching &output_matching);
private:
    static CheckResult checkMatch(const Signature &sign1, const Signature &sign2);
    static void partitionType1(Circuit *cir, std::vector<IOSet> &partition);
};
