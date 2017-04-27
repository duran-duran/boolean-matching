#include "signdata.h"

SignData::SignData(Circuit *cir)
{
    IOSupport output_support = IOSupportCalculator(cir).getOutputSupport();
    for (const auto &po : cir->getOutputs())
    {
        POSignature sign;
        sign.output_support = output_support.at(po);

        constexpr std::size_t max_iterations = 1000;
        sign.properties = Simulator(cir).simulate(max_iterations);

        signatures.insert({po, sign});
    }
}

POSignature SignData::getSignature(const std::string &po) const
{
    return signatures.at(po);
}
