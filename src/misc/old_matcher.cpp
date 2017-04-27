#include "matcher.h"

#include <algorithm>
#include <utility>
#include <limits>

Matcher::Matcher(Circuit *cir1, Circuit *cir2) :
    cir1(cir1), cir2(cir2)
{}

Matching Matcher::getResult()
{
    data1 = CircuitData(cir1);
    data2 = CircuitData(cir2);

    cur_match = Matching();

    while (true)
    {
        auto unmatched_signatures = getCommonUnmatchedSignatures();
        auto signs1 = data1.getUnmatchedSignatures(),
             signs2 = data2.getUnmatchedSignatures(),
             msigns1 = data1.getMatchedSignatures(),
             msigns2 = data2.getMatchedSignatures();
        while(!unmatched_signatures.empty())
        {
            Signature min_common_sign = *(std::min_element(unmatched_signatures.begin(), unmatched_signatures.end()));
            auto po_cluster1 = data1.unmatched_output_clusters.at(min_common_sign),
                 po_cluster2 = data2.unmatched_output_clusters.at(min_common_sign);

            auto rand_po1_it = po_cluster1.begin(),
                 rand_po2_it = po_cluster2.begin();
            std::advance(rand_po1_it, rand() % po_cluster1.size());
            std::advance(rand_po2_it, rand() % po_cluster2.size());

            match(*rand_po1_it, *rand_po2_it);

            unmatched_signatures = getCommonUnmatchedSignatures();
            signs1 = data1.getUnmatchedSignatures();
            signs2 = data2.getUnmatchedSignatures();
            msigns1 = data1.getMatchedSignatures();
            msigns2 = data2.getMatchedSignatures();
        }

        auto signs = getMinDiffSignatures();
        if (signs.first.empty() && signs.second.empty())
            break;

        auto unmatched_po_cluster = data2.unmatched_output_clusters.at(signs.second);
        auto unmatched_po_it = unmatched_po_cluster.begin();
        std::advance(unmatched_po_it, rand() % unmatched_po_cluster.size());
        auto stuck_inputs2 = data2.stickUnmatchedInputs(*unmatched_po_it, signs.second.size() - signs.first.size());
        cur_match.stuck_inputs2.insert(stuck_inputs2.begin(), stuck_inputs2.end());
    }

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

std::pair<Signature, Signature> Matcher::getMinDiffSignatures()
{
    auto unmatched_signs1 = data1.getUnmatchedSignatures(),
         unmatched_signs2 = data2.getUnmatchedSignatures();

    Signature sign1 = "",
              sign2 = "";
    size_t min_diff = std::numeric_limits<size_t>::max();

    for (const auto& lhs : unmatched_signs1)
    {
        for (const auto& rhs : unmatched_signs2)
        {
            if (getPrefix(lhs) == getPrefix(rhs) && (rhs.size() > lhs.size()))
            {
                size_t diff = rhs.size() - lhs.size();
                if (diff < min_diff)
                {
                    min_diff = diff;
                    sign1 = lhs;
                    sign2 = rhs;
                }
            }
        }
    }

    return std::make_pair(sign1, sign2);
}

void Matcher::match(const std::string &po1, const std::string &po2)
{
    if (data1.output_signatures.at(po1) != data2.output_signatures.at(po2))
        return;

    auto new_io_clusters1 = data1.match(po1),
         new_io_clusters2 = data2.match(po2);
    if (!new_io_clusters1.first.empty() && new_io_clusters2.first.empty())
        cur_match.input_matching.push_back(std::make_pair(new_io_clusters1.first, new_io_clusters2.first));
    cur_match.output_matching.push_back(std::make_pair(new_io_clusters1.second, new_io_clusters2.second));

    constexpr size_t match_score = 10,
                     bridging_score = 1;
    cur_match.score += match_score + (new_io_clusters2.second.size() - 1) * bridging_score;
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
    for (const auto &cluster : unmatched_output_clusters)
        result.insert(cluster.first);
    return result;
}

std::set<Signature> CircuitData::getMatchedSignatures()
{
    std::set<Signature> result;
    for (const auto &cluster : matched_output_clusters)
        result.insert(cluster.first);
    return result;
}

std::pair<IOSet, IOSet> CircuitData::match(const std::string &po)
{
    if (unmatched_outputs.find(po) == unmatched_outputs.end())
        return std::make_pair(IOSet(), IOSet());

    matched_outputs.insert(po);
    unmatched_outputs.erase(po);

    auto inputs = getUnmatchedInputs(po);

    matched_inputs.insert(inputs.begin(), inputs.end());
    for (const auto& pi : inputs)
        unmatched_inputs.erase(pi);

    constexpr char unmatched_id = '0';
    const char new_cluster_id = unmatched_id + getMatchedSignatures().size() + 1;

    Signature old_po_sign = output_signatures.at(po),
              new_po_sign = updateSignature(old_po_sign, new_cluster_id, inputs.size());
    output_signatures.at(po) = new_po_sign;

    if (matched_output_clusters.find(new_po_sign) == matched_output_clusters.end())
        matched_output_clusters[new_po_sign] = IOSet();
    matched_output_clusters.at(new_po_sign).insert(po);

    unmatched_output_clusters.at(old_po_sign).erase(po);
    if (unmatched_output_clusters.at(old_po_sign).empty())
        unmatched_output_clusters.erase(old_po_sign);

    for (const auto &uo : unmatched_outputs)
    {
        auto uo_support = output_support.at(uo);
        Signature old_sign = output_signatures.at(uo),
                  new_sign = old_sign;
        for (const auto &pi : inputs)
        {
            if (uo_support.find(pi) != uo_support.end())
                new_sign = updateSignature(new_sign, new_cluster_id, 1);
        }
        if (new_sign != old_sign)
        {
            output_signatures.at(uo) = new_sign;

            //if (new_sign == new_po_sign)
            if (new_sign == new_po_sign)
                matched_output_clusters.at(new_po_sign).insert(uo);
            else
            {
                if (unmatched_output_clusters.find(new_sign) == unmatched_output_clusters.end())
                    unmatched_output_clusters[new_sign] = IOSet();
                unmatched_output_clusters.at(new_sign).insert(uo);
            }

            unmatched_output_clusters.at(old_sign).erase(uo);
            if (unmatched_output_clusters.at(old_sign).empty())
                unmatched_output_clusters.erase(old_sign);
        }
    }

    return std::make_pair(inputs, matched_output_clusters.at(new_po_sign));
}

IOSet CircuitData::stickUnmatchedInputs(const std::string &po, size_t n)
{
    IOSet stuck_inputs;
    //careful here
    for (size_t i = 0; i < n; ++i)
    {
        auto inputs = getUnmatchedInputs(po);
        auto rand_pi_it = inputs.begin();
        std::advance(rand_pi_it, rand() % inputs.size());

        auto pi = *rand_pi_it;
        stuck_inputs.insert(pi);

        for (const auto &po : input_support.at(pi))
        {
            output_support.at(po).erase(pi);
            Signature old_sign = output_signatures.at(po),
                      new_sign = old_sign.substr(0, old_sign.length() - 1);
            output_signatures.at(po) = new_sign;

            if (unmatched_output_clusters.find(new_sign) == unmatched_output_clusters.end())
                unmatched_output_clusters[new_sign] = IOSet();
            unmatched_output_clusters.at(new_sign).insert(po);

            unmatched_output_clusters.at(old_sign).erase(po);
            if (unmatched_output_clusters.at(old_sign).empty())
                unmatched_output_clusters.erase(old_sign);
        }

        unmatched_inputs.erase(pi);
        input_support.erase(pi);
        //input_signatures.erase(pi);
        //input clusters ?
    }
    return stuck_inputs;
}

void CircuitData::calculateInitClusters()
{
    for (const auto &sup : output_support)
    {
        Signature sign(sup.second.size(), '0');
        output_signatures[sup.first] = sign;
        if (unmatched_output_clusters.find(sign) == unmatched_output_clusters.end())
            unmatched_output_clusters[sign] = IOSet();
        unmatched_output_clusters[sign].insert(sup.first);
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

Signature updateSignature(const Signature &old_sign, char cluster_id, size_t n)
{
    constexpr char unmatched_id = '0';
    Signature new_sign = old_sign;
    size_t start = new_sign.find(unmatched_id);
    for (size_t i = start; i < start + n; ++i)
        new_sign[i] = cluster_id;
    return new_sign;
}

std::string getPrefix(Signature sign)
{
    constexpr char unmatched_id = '0';
    return sign.substr(0, sign.find(unmatched_id));
}
