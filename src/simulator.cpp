#include "simulator.h"
#include "utils.h"
#include "checker.h"
#include <sstream>
#include <algorithm>

const PropertySet Simulator::all_properties =
{
    Property::PosUnate,
    Property::NegUnate
};

const SymmetrySet Simulator::all_symmetries =
{
    Symmetry::NESymmetry,
    Symmetry::ESymmetry
};

Simulator::Simulator(Circuit *cir) :
    cir(cir)
{}

PropertyMap Simulator::simulate(std::size_t max_iterations)
{
    PropertyMap input_properties;

    const std::string po = cir->getOutputs().front(); //only one output in cone
    for (const auto &pi : cir->getInputs())
    {
        input_properties.insert({pi, all_properties});
        for (std::size_t it = 0; it < max_iterations; ++it)
        {
            auto vec_pair = generateDisjointPair({pi});
//            log("Input vector 1 : %s", inVecToStr(vec_pair.first).c_str());
//            log("Input vector 2 : %s", inVecToStr(vec_pair.second).c_str());
            checkRemoval(input_properties.at(pi),
                         vec_pair.first.at(pi), vec_pair.second.at(pi),
                         cir->evalOutput(po, vec_pair.first), cir->evalOutput(po, vec_pair.second));

            if (input_properties.empty())
                break;
        }
        confirmProperties(pi, input_properties.at(pi));
    }

    return std::move(input_properties);
}

SymmetryMap Simulator::simulateSym(std::size_t max_iterations)
{
    if (cir->getInputs().size() < 2)
        return {};

    SymmetryMap input_symmetries;

    const std::string po = cir->getOutputs().front(); //only one output in cone

    for (std::size_t i = 0; i < cir->getInputs().size() - 1; ++i)
    {
        const auto& pi1 = cir->getInputs()[i];
        for (std::size_t j = i + 1; j < cir->getInputs().size(); ++j)
        {
            const auto &pi2 = cir->getInputs()[j];
//            if (pi1 == pi2)
//                continue;

            input_symmetries.insert({std::make_pair(pi1, pi2), all_symmetries});
            for (std::size_t it = 0; it < max_iterations; ++it)
            {
                auto vec_pair = generateDisjointPair({pi1, pi2});
                //            log("Input vector 1 : %s", inVecToStr(vec_pair.first).c_str());
                //            log("Input vector 2 : %s", inVecToStr(vec_pair.second).c_str());
                checkRemoval(input_symmetries.at(std::make_pair(pi1, pi2)),
                             vec_pair.first.at(pi1), vec_pair.first.at(pi2),
                             vec_pair.second.at(pi1), vec_pair.second.at(pi2),
                             cir->evalOutput(po, vec_pair.first), cir->evalOutput(po, vec_pair.second));

                if (input_symmetries.at(std::make_pair(pi1, pi2)).empty())
                    break;
            }
            confirmSymmetries(pi1, pi2, input_symmetries.at(std::make_pair(pi1, pi2)));
        }
    }
    return std::move(input_symmetries);
}

SVSymmetryMap Simulator::simulateSVSym(std::size_t max_iterations)
{
    SVSymmetryMap sv_symmetries;
    const std::string po = cir->getOutputs().front(); //only one output in cone
    for (const auto &pi1 : cir->getInputs())
    {
        for (const auto &pi2 : cir->getInputs())
        {
            if (pi2 == pi1)
                continue;

            sv_symmetries[pi1].insert(std::make_pair(pi2, true));
            sv_symmetries[pi1].insert(std::make_pair(pi2, false));
        }

        for (std::size_t it = 0; it < max_iterations; ++it)
        {
            auto vec_pair = generateDisjointPair({pi1});
            //            log("Input vector 1 : %s", inVecToStr(vec_pair.first).c_str());
            //            log("Input vector 2 : %s", inVecToStr(vec_pair.second).c_str());
            checkRemoval(sv_symmetries.at(pi1),
                         pi1, vec_pair.first,
                         cir->evalOutput(po, vec_pair.first), cir->evalOutput(po, vec_pair.second));

            if (sv_symmetries.at(pi1).empty())
                break;
        }
    }
    return std::move(sv_symmetries);
}

std::pair<InVector, InVector> Simulator::generateDisjointPair(const std::vector<std::string> &disjoint_inputs) const
{
    const auto &inputs = cir->getInputs();

    InVector vector1, vector2;

    for (const auto &i : inputs)
    {
        bool value = (rand() % 2) == 0;
        vector1.insert({i, value});
        if (std::find(disjoint_inputs.begin(), disjoint_inputs.end(), i) == disjoint_inputs.end())
            vector2.insert({i, value});
        else
            vector2.insert({i, !value});
    }

    return std::make_pair(vector1, vector2);
}

void Simulator::checkRemoval(PropertySet &properties, bool in_value1, bool in_value2, bool out_value1, bool out_value2)
{
    if (out_value1 == out_value2 || in_value1 == in_value2) //checks only on disjoint input & output values
        return;

    auto properties_copy = properties;
    for (auto prop : properties_copy)
    {
        bool erase = false;

        switch (prop)
        {
        case Property::PosUnate:
            erase = out_value1 && !in_value1;
            break;
        case Property::NegUnate:
            erase = out_value1 && in_value1;
            break;
        }

        if (erase)
        {
            log("Erasing property %s", propToStr(prop).c_str());
            properties.erase(prop);
        }
    }
}

void Simulator::checkRemoval(SymmetrySet &symmetries, bool in_value11, bool in_value12, bool in_value21, bool in_value22, bool out_value1, bool out_value2)
{
    if (out_value1 == out_value2) //checks only on disjoint input & output values
        return;

    auto symmetries_copy = symmetries;
    for (auto sym : symmetries_copy)
    {
        bool erase = false;

        switch (sym)
        {
        case Symmetry::NESymmetry:
            erase = in_value11 && !in_value12;
            break;
        case Symmetry::ESymmetry:
            erase = in_value11 && in_value12;
            break;
        }

        if (erase)
        {
            log("Erasing symmetry %s", symToStr(sym).c_str());
            symmetries.erase(sym);
        }
    }
}

void Simulator::checkRemoval(SVSymmetrySet &sv_symmetries, const std::string &pi, const InVector &in_vec, bool out_value1, bool out_value2)
{
    if (out_value1 == out_value2) //checks only on disjoint input & output values
        return;

    auto sv_symmetries_copy = sv_symmetries;
    for (auto sv_sym : sv_symmetries_copy)
    {
        if (sv_sym.first != pi && in_vec.at(sv_sym.first) == sv_sym.second)
        {
            log("Erasing sv-symmetry %s", svSymToStr(sv_sym).c_str());
            sv_symmetries.erase(sv_sym);
        }
    }
}

void Simulator::confirmProperties(const std::string &pi, PropertySet &properties) const
{
    const auto &po = cir->getOutputs().front();
    auto properties_copy = properties;

    for (Property property : properties_copy)
    {
        bool unsat = false;
        switch (property)
        {
        case Property::PosUnate:
        {
            Circuit* neg_cofactor = new Circuit(*cir);
            neg_cofactor->stuckInput(pi, false);
            neg_cofactor->print();

            Circuit *inv_pos_cofactor = new Circuit(*cir);
            inv_pos_cofactor->stuckInput(pi, true);
            inv_pos_cofactor->invertOutput(po);
            inv_pos_cofactor->print();

            Circuit *product_miter = Circuit::getMiter(neg_cofactor, inv_pos_cofactor, FUNCTION_AND);
            product_miter->print();
            unsat = checkMiter(product_miter);

            delete neg_cofactor;
            delete inv_pos_cofactor;
            delete product_miter;

            break;
        }
        case Property::NegUnate:
        {
            Circuit* inv_neg_cofactor = new Circuit(*cir);
            inv_neg_cofactor->stuckInput(pi, false);
            inv_neg_cofactor->invertOutput(po);

            Circuit *pos_cofactor = new Circuit(*cir);
            pos_cofactor->stuckInput(pi, true);

            Circuit *product_miter = Circuit::getMiter(inv_neg_cofactor, pos_cofactor, FUNCTION_AND);
            product_miter->print();
            unsat = checkMiter(product_miter);

            delete inv_neg_cofactor;
            delete pos_cofactor;
            delete product_miter;
            break;
        }
        default:
            break;
        }

        if (unsat)
        {
            log("Property %s confirmed for input %s", propToStr(property).c_str(), pi.c_str());
        }
        else
        {
            log ("Property %s was not confirmed for input %s, erasing", propToStr(property).c_str(), pi.c_str());
            properties.erase(property);
        }
    }
}

void Simulator::confirmSymmetries(const std::string &pi1, const std::string &pi2, SymmetrySet &symmetries) const
{
    const auto &po = cir->getOutputs().front();
    auto symmetries_copy = symmetries;

    for (Symmetry symmetry : symmetries_copy)
    {
        bool unsat = false;
        switch (symmetry)
        {
        case Symmetry::NESymmetry:
        {
            Circuit* cofactor1 = new Circuit(*cir);
            cofactor1->stuckInput(pi1, false);
            cofactor1->stuckInput(pi2, true);
            cofactor1->print();

            Circuit *cofactor2 = new Circuit(*cir);
            cofactor2->stuckInput(pi1, true);
            cofactor2->stuckInput(pi2, false);
            cofactor2->print();

            Circuit *product_miter = Circuit::getMiter(cofactor1, cofactor2);
            product_miter->print();
            unsat = checkMiter(product_miter);

            delete cofactor1;
            delete cofactor2;
            delete product_miter;

            break;
        }
        case Symmetry::ESymmetry:
        {
            Circuit* cofactor1 = new Circuit(*cir);
            cofactor1->stuckInput(pi1, false);
            cofactor1->stuckInput(pi2, false);
            cofactor1->invertOutput(po);

            Circuit *cofactor2 = new Circuit(*cir);
            cofactor2->stuckInput(pi1, true);
            cofactor2->stuckInput(pi2, true);

            Circuit *product_miter = Circuit::getMiter(cofactor1, cofactor2);
            product_miter->print();
            unsat = checkMiter(product_miter);

            delete cofactor1;
            delete cofactor2;
            delete product_miter;
            break;
        }
        default:
            break;
        }

        if (unsat)
        {
            log("Symmetry %s confirmed for inputs %s, %s", symToStr(symmetry).c_str(), pi1.c_str(), pi2.c_str());
        }
        else
        {
            log ("Symmetry %s was not confirmed for inputs %s, %s, erasing", symToStr(symmetry).c_str(), pi1.c_str(), pi2.c_str());
            symmetries.erase(symmetry);
        }
    }
}

void Simulator::confirmSymmetries(const std::string &pi, SVSymmetrySet &sv_symmetries) const
{
    auto sv_symmetries_copy = sv_symmetries;

    for (const auto &sv_sym : sv_symmetries_copy)
    {
        bool unsat = false;

        Circuit* pos_cofactor = new Circuit(*cir);
        pos_cofactor->stuckInput(pi, true);
        pos_cofactor->stuckInput(sv_sym.first, sv_sym.second);
        pos_cofactor->print();

        Circuit *neg_cofactor = new Circuit(*cir);
        neg_cofactor->stuckInput(pi, false);
        neg_cofactor->stuckInput(sv_sym.first, sv_sym.second);
        neg_cofactor->print();

        Circuit *product_miter = Circuit::getMiter(pos_cofactor, neg_cofactor);
        product_miter->print();
        unsat = checkMiter(product_miter);

        delete pos_cofactor;
        delete neg_cofactor;
        delete product_miter;

        if (unsat)
        {
            log("SV-Symmetry %s confirmed for input %s", svSymToStr(sv_sym).c_str(), pi.c_str());
        }
        else
        {
            log ("SV-Symmetry %s was not confirmed for input %s, erasing", svSymToStr(sv_sym).c_str(), pi.c_str());
            sv_symmetries.erase(sv_sym);
        }
    }
}

std::string propToStr(Property prop)
{
    switch (prop)
    {
    case Property::PosUnate:
        return "PosUnate";
    case Property::NegUnate:
        return "NegUnate";
    default:
        return "";
    }
}

std::string propSetToStr(const PropertySet &properties)
{
    std::stringstream ss;
    for (auto prop : properties)
        ss << propToStr(prop) << " ";
    return ss.str();
}

std::string inVecToStr(const InVector &in_vec)
{
    std::stringstream ss;
    for (const auto& it : in_vec)
    {
        ss << it.first << ": ";
        ss << (it.second ? 1 : 0) << " ";
    }
    return ss.str();
}

std::string symToStr(Symmetry sym)
{
    switch (sym)
    {
    case Symmetry::NESymmetry:
        return "NESymmetry";
    case Symmetry::ESymmetry:
        return "ESymmetry";
    default:
        return "";
    }
}

std::string symSetToStr(const SymmetrySet &symmetries)
{
    std::string result;
    for (auto sym : symmetries)
        result += symToStr(sym) + " ";
    return result;
}

std::string svSymToStr(const std::pair<std::string, bool> &sv_sym)
{
    return (sv_sym.second ? "" : "not-") + sv_sym.first;
}

std::string svSymSetToStr(const SVSymmetrySet &sv_symmetries)
{
    std::string result;
    for (const auto &sv_sym : sv_symmetries)
        result += svSymToStr(sv_sym) + " ";
    return std::move(result);
}
