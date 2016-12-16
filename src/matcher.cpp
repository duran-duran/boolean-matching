#include "matcher.h"

#include <algorithm>
#include <utility>

Matcher::Matcher(Circuit *cir1, Circuit *cir2) :
    cir1(cir1), cir2(cir2)
{}

Matching Matcher::getResult()
{
    data1 = CircuitData(cir1);
    data2 = CircuitData(cir2);

    cur_match = Matching();

//    while (!data2.unmatched_outputs.empty())
//    {
        auto unmatched_signatures = getCommonUnmatchedSignatures();
        while(!unmatched_signatures.empty())
        {
            Signature min_common_sign = *(std::min_element(unmatched_signatures.begin(), unmatched_signatures.end()));
            auto po_cluster1 = data1.output_clusters.at(min_common_sign),
                 po_cluster2 = data2.output_clusters.at(min_common_sign);

            auto rand_po1_it = po_cluster1.begin(),
                 rand_po2_it = po_cluster2.begin();
            std::advance(rand_po1_it, rand() % po_cluster1.size());
            std::advance(rand_po2_it, rand() % po_cluster2.size());

            match(*rand_po1_it, *rand_po2_it);

            unmatched_signatures = getCommonUnmatchedSignatures();
        }
//    }

    return cur_match;
}

std::set<Signature> Matcher::getCommonUnmatchedSignatures()
{
    auto unmatched_signs1 = data1.getUnmatchedSignatures(),
         unmatched_signs2 = data2.getUnmatchedSignatures();
    std::set<Signature> common_unmatched_signs;
    std::set_intersection(unmatched_signs1.begin(), unmatched_signs1.end(),
                          unmatched_signs2.begin(), unmatched_signs2.end(),
                          std::inserter(common_unmatched_signs, common_unmatched_signs.begin()));
    return common_unmatched_signs;
}

void Matcher::match(const std::string &po1, const std::string &po2)
{
    if (data1.output_signatures.at(po1) != data2.output_signatures.at(po2))
        return;

    cur_match.output_matching.push_back(std::make_pair(IOSet{po1}, IOSet{po2}));

    auto input_cluster1 = data1.match(po1),
         input_cluster2 = data2.match(po2);
    cur_match.input_matching.push_back(std::make_pair(input_cluster1, input_cluster2));

    constexpr size_t match_score = 10;
    cur_match.score += match_score;
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

std::set<Signature> CircuitData::getUnmatchedSignatures()
{
    std::set<Signature> result;
    for (const auto &cluster : output_clusters)
    {
        if (cluster.first.find('0') != std::string::npos)
            result.insert(cluster.first);
    }
    return result;
}

std::set<Signature> CircuitData::getMatchedSignatures()
{
    std::set<Signature> result;
    for (const auto &cluster : output_clusters)
    {
        if (cluster.first.find('0') == std::string::npos)
            result.insert(cluster.first);
    }
    return result;
}

IOSet CircuitData::match(const std::string &po)
{
    if (unmatched_outputs.find(po) == unmatched_outputs.end())
        return IOSet();

    matched_outputs.insert(po);
    unmatched_outputs.erase(po);

    auto inputs = getUnmatchedInputs(po);

    matched_inputs.insert(inputs.begin(), inputs.end());
    for (const auto& pi : inputs)
        unmatched_inputs.erase(pi);

    constexpr char unmatched_id = '0';
    const char new_cluster_id = unmatched_id + getMatchedSignatures().size() + 1;

    {
        Signature old_sign = output_signatures.at(po),
                  new_sign = updateSignature(old_sign, new_cluster_id, inputs.size());
        output_signatures.at(po) = new_sign;

        if (output_clusters.find(new_sign) == output_clusters.end())
            output_clusters[new_sign] = IOSet();
        output_clusters.at(new_sign).insert(po);

        output_clusters.at(old_sign).erase(po);
        if (output_clusters.at(old_sign).empty())
            output_clusters.erase(old_sign);
    }

    for (const auto &uo : unmatched_outputs)
    {
        auto uo_support = output_support.at(uo);
        for (const auto &pi : inputs)
        {
            if (uo_support.find(pi) != uo_support.end())
            {
                Signature old_sign = output_signatures.at(uo),
                          new_sign = updateSignature(old_sign, new_cluster_id, 1);
                output_signatures.at(uo) = new_sign;

                if (output_clusters.find(new_sign) == output_clusters.end())
                    output_clusters[new_sign] = IOSet();
                output_clusters.at(new_sign).insert(uo);

                output_clusters.at(old_sign).erase(uo);
                if (output_clusters.at(old_sign).empty())
                    output_clusters.erase(old_sign);
            }
        }
    }

    return inputs;
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

IOSet CircuitData::getUnmatchedInputs(const std::string &po)
{
    IOSet result;
    for (const auto &pi : output_support.at(po))
    {
        if (unmatched_inputs.find(pi) != unmatched_inputs.end())
            result.insert(pi);
    }
    return result;
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
        else if (type == NET_DEFAULT || type == NET_OUTPUT)
        {
            auto addSupport = getSupport(i);
            supportCache.at(o).insert(addSupport.begin(), addSupport.end());
        }
    }
    return supportCache.at(o);
}

Signature updateSignature(const Signature &old_sign, char cluster_id, size_t n)
{
    constexpr char unmatched_id = '0';
    Signature new_sign = old_sign;
    size_t start = new_sign.find(unmatched_id);
    for (size_t i = start; i < start + n; ++i)
        new_sign[i] = cluster_id;
    return new_sign;
}
