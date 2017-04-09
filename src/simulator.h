#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "circuit.h"

enum class Property
{
    PosUnate,
    NegUnate
};

std::string inVecToStr(const InVector &in_vec);
std::string propToStr(Property prop);
std::string propSetToStr(const std::set<Property> &properties);

class Simulator
{
public:
    Simulator(Circuit *cir);

    std::map<std::string, std::set<Property>> simulate(std::size_t max_iterations);
private:
    Circuit *cir;

    std::pair<InVector, InVector> generateDisjointPair(const std::string &pi) const;
    static void checkRemoval(std::set<Property> &properties,
                             bool in_value1, bool in_value2,
                             bool out_value1, bool out_value2);

    static const std::set<Property> all_properties;
};

#endif // __SIMULATOR_H__
