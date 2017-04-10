#include "simulator.h"
#include "utils.h"
#include "checker.h"
#include <sstream>

const std::set<Property> Simulator::all_properties =
{
    Property::PosUnate,
    Property::NegUnate
};

Simulator::Simulator(Circuit *cir) :
    cir(cir)
{}

std::map<std::string, std::set<Property>> Simulator::simulate(std::size_t max_iterations)
{
    std::map<std::string, std::set<Property>> input_properties;

    const std::string po = cir->getOutputs().front(); //only one output in cone
    for (const auto &pi : cir->getInputs())
    {
        input_properties.insert({pi, all_properties});
        for (std::size_t it = 0; it < max_iterations; ++it)
        {
            auto vec_pair = generateDisjointPair(pi);
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
    return input_properties;
}

std::pair<InVector, InVector> Simulator::generateDisjointPair(const std::string &pi) const
{
    const auto &inputs = cir->getInputs();

    InVector vector1, vector2;

    for (const auto &i : inputs)
    {
        bool value = (rand() % 2) == 0;
        vector1.insert({i, value});
        vector2.insert({i, (i == pi) ? !value : value});
    }

    return std::make_pair(vector1, vector2);
}

void Simulator::checkRemoval(std::set<Property> &properties, bool in_value1, bool in_value2, bool out_value1, bool out_value2)
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

void Simulator::confirmProperties(const std::string &pi, std::set<Property> &properties) const
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

std::string propSetToStr(const std::set<Property> &properties)
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
