#pragma once

#include <unordered_map>
#include "circuit.h"
#include "support_calculator.h"
#include "simulator.h"

struct Signature
{
    IOSet output_support;
    PropertyMap properties;
};

class SignData
{
public:
    SignData(Circuit *cir);

    Signature getSignature(const std::string &po) const;
private:
    std::unordered_map<std::string, Signature> signatures;

};
