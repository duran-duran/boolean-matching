#pragma once

#include "circuit.h"

enum class Unateness
{
    Binate,
    PosUnate,
    NegUnate,
    Unknown
};

enum class Symmetry
{
    None,
    NESymmetry,
    ESymmetry,
    BothNEandE,
    Unknown
};

using IOSet = std::set<std::string>;

using UnatenessSet = std::set<Unateness>;
using UnatenessMap = std::map<std::string, UnatenessSet>;
using SymmetrySet = std::set<Symmetry>;
using SymmetryMap = std::map<std::pair<std::string, std::string>, SymmetrySet>;
using SymmetryPartition = std::vector<std::pair<Symmetry, IOSet>>;

using SVSymmetrySet = std::set<std::pair<std::string, bool>>;
using SVSymmetryMap = std::map<std::string, SVSymmetrySet>;

std::string inVecToStr(const InVector &in_vec);
std::string propToStr(Unateness prop);
std::string propSetToStr(const UnatenessSet &properties);
std::string symToStr(Symmetry sym);
std::string symSetToStr(const SymmetrySet &symmetries);
std::string svSymToStr(const std::pair<std::string, bool> &sv_sym);
std::string svSymSetToStr(const SVSymmetrySet &sv_symmetries);

class Simulator
{
public:
    Simulator(Circuit *cir);

    UnatenessMap simulate(std::size_t max_iterations);
    SymmetryPartition simulateSym(std::size_t max_iterations);
    SVSymmetryMap simulateSVSym(std::size_t max_iterations);
private:
    Circuit *cir;

    std::pair<InVector, InVector> generateDisjointPair(const std::vector<std::string> &disjoint_inputs) const;
    static void checkRemoval(UnatenessSet &properties,
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
    void confirmProperties(const std::string &pi, UnatenessSet &properties) const;
    void confirmSymmetries(const std::string &pi1, const std::string &pi2, SymmetrySet &symmetries) const;
    void confirmSymmetries(const std::string &pi, SVSymmetrySet &sv_symmetries) const;

    static const UnatenessSet all_properties;
    static const SymmetrySet all_symmetries;
};
