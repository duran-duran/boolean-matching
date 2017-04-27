#pragma once

#include <unordered_map>
#include "circuit.h"
#include "support_calculator.h"
#include "simulator.h"

struct POSignature
{
    IOSet output_support;
    UnatenessMap properties;
};

class SignData
{
public:
    SignData(Circuit *cir);

    POSignature getSignature(const std::string &po) const;
private:
    std::unordered_map<std::string, POSignature> signatures;

};
