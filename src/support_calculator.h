#pragma once

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
