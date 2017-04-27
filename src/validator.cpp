#include "validator.h"

ValidationResult Validator::validateMatching(Circuit *cir1, Circuit *cir2, const SignData &signs1, const SignData &signs2, const Matching &output_matching)
{
    ValidationResult result;

    for (const auto& match : output_matching)
    {
        if (checkMatch(signs1.getSignature(match.first), signs2.getSignature(match.second)).valid)
            result.valid_matches.insert(match);
        else
            result.invalid_matches.insert(match);
    }

    return std::move(result);
}

CheckResult Validator::checkMatch(const Signature &sign1, const Signature &sign2)
{
    if (sign1.output_support.size() != sign2.output_support.size())
        return CheckResult{false, {}};

    CheckResult result;

    auto splitByUnateness = [](const PropertyMap &property_map)
    {
        IOSet pos_unate_cluster, neg_unate_cluster, binate_cluster, undefined_cluster;
        for (const auto& it : property_map)
        {
            const auto &pi = it.first;
            const auto &properties = it.second;

            if (properties.find(Property::PosUnate) != properties.end() &&
                properties.find(Property::NegUnate) == properties.end())
            {
                pos_unate_cluster.insert(pi);
            }
            else if (properties.find(Property::PosUnate) == properties.end() &&
                     properties.find(Property::NegUnate) != properties.end())
            {
                neg_unate_cluster.insert(pi);
            }
            else if (properties.find(Property::PosUnate) == properties.end() &&
                     properties.find(Property::NegUnate) == properties.end())
            {
                binate_cluster.insert(pi);
            }
            else
            {
                undefined_cluster.insert(pi);
            }
        }

        return std::vector<IOSet>{pos_unate_cluster, neg_unate_cluster, binate_cluster, undefined_cluster};
    };

    std::vector<IOSet> input_clusters1 = splitByUnateness(sign1.properties),
                       input_clusters2 = splitByUnateness(sign2.properties);

    constexpr std::size_t clustersCnt = 4;
    for (std::size_t i = 0; i < clustersCnt; ++i)
    {
        if (input_clusters1[i].size() != input_clusters2[i].size())
            return {false, {}};
        else
            result.inputMatching.push_back(std::make_pair(input_clusters1[i], input_clusters2[i]));
    }

    result.valid = true;

    return std::move(result);
}

std::vector<IOSet> Validator::partitionType1(Circuit *cir, const std::vector<IOSet> &partition)
{
    std::vector<IOSet> new_partition = partition;
    const auto &po = cir->getOutputs().front();
    constexpr std::size_t max_it = 10000;
    for (std::size_t it = 0; it < max_it && new_partition.size() == partition.size(); ++it)
    {
        new_partition.clear();
        for (std::size_t i = 0; i < partition.size(); ++i)
        {
            if (partition.size() == 1)
                continue;

            InVector in_vec;
            for (std::size_t j = 0; j < partition.size(); ++j)
            {
                bool value = (rand() % 2) == 0;
                for (const auto &pi : partition[j])
                    in_vec.insert({pi, value});
            }
            IOSet set0, set1;
            for (const auto &pi : partition[i])
            {
                InVector in_vec_flip = in_vec;
                in_vec_flip.at(pi) = !in_vec_flip.at(pi);
                if (cir->evalOutput(po, in_vec_flip))
                    set1.insert(pi);
                else
                    set0.insert(pi);
            }
            if (!set0.empty())
                new_partition.push_back(set0);
            if (!set1.empty())
                new_partition.push_back(set1);
        }
    }
    return std::move(new_partition);
}

std::vector<IOSet> Validator::partitionType2(Circuit *cir, const std::vector<IOSet> &partition)
{
    std::vector<IOSet> new_partition = partition;
    const auto &po = cir->getOutputs().front();
    constexpr std::size_t max_it = 10000;
    for (std::size_t it = 0; it < max_it && new_partition.size() == partition.size(); ++it)
    {
        new_partition.clear();
        for (std::size_t i = 0; i < partition.size(); ++i)
        {
            if (partition.size() == 1)
                continue;

            InVector in_vec;
            for (std::size_t j = 0; j < partition.size(); ++j)
            {
                bool value = (rand() % 2) == 0;
                for (const auto &pi : partition[j])
                    in_vec.insert({pi, value});
            }
            std::map<int, IOSet> val_sets;
            for (const auto &pi1 : partition[i])
            {
                InVector in_vec_flip = in_vec;
                in_vec_flip.at(pi1) = !in_vec_flip.at(pi1);
                int output_weight = 0;
                for (const auto &pi2 : partition[i])
                {
                    if (pi1 == pi2)
                        continue;

                    InVector new_in_vec = in_vec_flip;
                    new_in_vec.at(pi2) = !new_in_vec.at(pi2);
                    if (cir->evalOutput(po, new_in_vec))
                        ++output_weight;
                }
                val_sets[output_weight].insert(pi1);
            }
            for (const auto &set : val_sets)
                new_partition.push_back(set.second);;
        }
    }
    return std::move(new_partition);
}
