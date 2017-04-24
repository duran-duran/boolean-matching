#include "support_calculator.h"

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
        else if (type == NET_DEFAULT || type == NET_OUTPUT)
        {
            auto addSupport = getSupport(i);
            supportCache.at(o).insert(addSupport.begin(), addSupport.end());
        }
    }
    return supportCache.at(o);
}
