#include "matcher.h"

Matcher::Matcher(Circuit *cir1, Circuit *cir2) :
    cir1(cir1), cir2(cir2)
{}

Matching Matcher::getResult()
{
    data1 = CircuitData(cir1);
    data2 = CircuitData(cir2);

    return Matching(data1.output_clusters, data1.input_clusters,
                    data2.output_clusters, data2.input_clusters,
                    0);
}

CircuitData::CircuitData()
{}

CircuitData::CircuitData(Circuit *cir) :
    unmatched_inputs(cir->getInputs().begin(), cir->getInputs().end()),
    unmatched_outputs(cir->getOutputs().begin(), cir->getOutputs().end()),
    output_support(IOSupportCalculator(cir).getOutputSupport()),
    input_support(IOSupportCalculator::swapSupport(output_support))
{
    calculateInitClusters();
}

void CircuitData::calculateInitClusters()
{
    for (const auto &sup : output_support)
    {
        Signature sign(sup.second.size(), '0');
        output_signatures[sup.first] = sign;
        if (output_clusters.find(sign) == output_clusters.end())
            output_clusters[sign] = IOSet();
        output_clusters[sign].insert(sup.first);
    }
}

IOSupportCalculator::IOSupportCalculator(Circuit *cir) :
    cir(cir)
{}

IOSupport IOSupportCalculator::swapSupport(const IOSupport &support)
{
    IOSupport swappedSupport;
    for (const auto& sup : support)
    {
        for (const auto& io : sup.second)
        {
            if (swappedSupport.find(io) == swappedSupport.end())
                swappedSupport[io] = IOSet();
            swappedSupport[io].insert(sup.first);
        }
    }
    return swappedSupport;
}

IOSupport IOSupportCalculator::getOutputSupport()
{
    supportCache.clear();

    IOSupport result;
    for (const auto &po : cir->getOutputs())
        result[po] = getSupport(po);
    return result;
}

IOSet IOSupportCalculator::getSupport(const std::string &o)
{
    if (supportCache.find(o) != supportCache.end())
        return supportCache.at(o);

    supportCache[o] = IOSet();
    for (const auto& i : cir->getNetInput(o)->input_names)
    {
        auto type = cir->getNetType(i);
        if (type == NET_INPUT)
        {
            supportCache.at(o).insert(i);
        }
        else if (type == NET_DEFAULT)
        {
            auto addSupport = getSupport(i);
            supportCache.at(o).insert(addSupport.begin(), addSupport.end());
        }
    }
    return supportCache.at(o);
}
