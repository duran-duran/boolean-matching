#pragma once

#include "circuit.h"

enum class Property
{
    PosUnate,
    NegUnate
};

enum class Symmetry
{
    NESymmetry,
    ESymmetry
};

using PropertySet = std::set<Property>;
using PropertyMap = std::map<std::string, PropertySet>;
using SymmetrySet = std::set<Symmetry>;
using SymmetryMap = std::map<std::pair<std::string, std::string>, SymmetrySet>;

using SVSymmetrySet = std::set<std::pair<std::string, bool>>;
using SVSymmetryMap = std::map<std::string, SVSymmetrySet>;

std::string inVecToStr(const InVector &in_vec);
std::string propToStr(Property prop);
std::string propSetToStr(const PropertySet &properties);
std::string symToStr(Symmetry sym);
std::string symSetToStr(const SymmetrySet &symmetries);
std::string svSymToStr(const std::pair<std::string, bool> &sv_sym);
std::string svSymSetToStr(const SVSymmetrySet &sv_symmetries);

class Simulator
{
public:
    Simulator(Circuit *cir);

    PropertyMap simulate(std::size_t max_iterations);
    SymmetryMap simulateSym(std::size_t max_iterations);
    SVSymmetryMap simulateSVSym(std::size_t max_iterations);
private:
    Circuit *cir;

    std::pair<InVector, InVector> generateDisjointPair(const std::vector<std::string> &disjoint_inputs) const;
    static void checkRemoval(PropertySet &properties,
                             bool in_value1, bool in_value2,
                             bool out_value1, bool out_value2);
    static void checkRemoval(SymmetrySet &symmetries,
                             bool in_value11, bool in_value12,
                             bool in_value21, bool in_value22,
                             bool out_value1, bool out_value2);
    static void checkRemoval(SVSymmetrySet &sv_symmetries,
                             const std::string &pi,
                             const InVector &in_vec,
                             bool out_value1, bool out_value2);
    void confirmProperties(const std::string &pi, PropertySet &properties) const;
    void confirmSymmetries(const std::string &pi1, const std::string &pi2, SymmetrySet &symmetries) const;
    void confirmSymmetries(const std::string &pi, SVSymmetrySet &sv_symmetries) const;

    static const PropertySet all_properties;
    static const SymmetrySet all_symmetries;
};
