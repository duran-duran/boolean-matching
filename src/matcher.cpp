#include "matcher.h"
#include "utils.h"

Matcher::Matcher(Circuit *cir1, Circuit *cir2) :
    cir1(cir1), cir2(cir2)
{
    for (const auto &po : cir1->getOutputs())
        cones1.insert({po, cir1->getCone(po)});
    for (const auto &po : cir2->getOutputs())
        cones2.insert({po, cir2->getCone(po)});

    cir1_po_partition = { {POSignature(cir1), IOSet(cir1->getOutputs().begin(), cir1->getOutputs().end())} };
    cir2_po_partition = { {POSignature(cir1), IOSet(cir2->getOutputs().begin(), cir2->getOutputs().end())} };

    for (const auto& po :cir1->getOutputs())
        cir1_pi_partitions.insert({po, { {PISignature(), IOSet(cones1.at(po)->getInputs().begin(), cones1.at(po)->getInputs().end())} } });
    for (const auto& po :cir2->getOutputs())
        cir2_pi_partitions.insert({po, { {PISignature(), IOSet(cones2.at(po)->getInputs().begin(), cones2.at(po)->getInputs().end())} } });
}

Matcher::~Matcher()
{
    for (const auto &it : cones1)
        delete it.second;
    for (const auto &it : cones2)
        delete it.second;
}

Matcher &Matcher::splitBySupport()
{
    splitBySupport(cir1_po_partition, cir1_pi_partitions, cir1, cones1);
    splitBySupport(cir2_po_partition, cir2_pi_partitions, cir2, cones2);
    return *this;
}

Matcher &Matcher::splitByUnateness()
{
    splitByUnateness(cir1_po_partition, cir1_pi_partitions, cir1, cones1);
    splitByUnateness(cir2_po_partition, cir2_pi_partitions, cir2, cones2);
    return *this;
}

Matcher &Matcher::splitBySymmetry()
{
    splitBySymmetry(cir1_po_partition, cir1_pi_partitions, cir1, cones1);
    splitBySymmetry(cir2_po_partition, cir2_pi_partitions, cir2, cones2);
    return *this;
}

Matcher &Matcher::splitBySimType1(std::size_t max_it)
{
    for (std::size_t it = 0; it < max_it; ++it)
    {
        if (splitBySimType1())
            it = 0;
    }
    return *this;
}

Matcher &Matcher::splitBySimType2(std::size_t max_it)
{
    for (std::size_t it = 0; it < max_it; ++it)
    {
        if (splitBySimType2())
            it = 0;
    }
    return *this;
}

bool Matcher::splitBySimType1()
{
    POPartition new_partition1 = cir1_po_partition,
                new_partition2 = cir2_po_partition;

    bool split = false;

    for (const auto &cluster : cir1_po_partition)
    {
        if (split || (cir2_po_partition.find(cluster.first) == cir2_po_partition.end()))
            continue;

        POSignature sign = cluster.first;
        new_partition1.erase(sign);
        new_partition2.erase(sign);

        for (std::size_t i = 0; i < sign.input_signatures.size(); ++i)
        {
            std::vector<bool> boolVec;
            for (std::size_t j = 0; j < sign.input_signatures.size(); ++j)
            {
                bool value = (rand() % 2) == 0;
                for (std::size_t k = 0; k < sign.input_signatures[j].first; ++k)
                    boolVec.push_back(value);
            }

            for (const auto &po : cir1_po_partition.at(sign))
            {
                auto result = splitBySimType1(po, cones1, cir1_pi_partitions.at(po), boolVec, i);
                new_partition1[POSignature(cir1_pi_partitions.at(po))].insert(po);
                split = split || result;
            }
            for (const auto &po : cir2_po_partition.at(sign))
            {
                auto result = splitBySimType1(po, cones2, cir2_pi_partitions.at(po), boolVec, i);
                new_partition2[POSignature(cir2_pi_partitions.at(po))].insert(po);
                split = split || result;
            }
            if (split)
                break;
        }
    }
    cir1_po_partition = new_partition1;
    cir2_po_partition = new_partition2;
    return split;
}

bool Matcher::splitBySimType2()
{
    POPartition new_partition1 = cir1_po_partition,
                new_partition2 = cir2_po_partition;

    bool split = false;

    for (const auto &cluster : cir1_po_partition)
    {
        if (split || (cir2_po_partition.find(cluster.first) == cir2_po_partition.end()))
            continue;

        POSignature sign = cluster.first;
        new_partition1.erase(sign);
        new_partition2.erase(sign);

        for (std::size_t i = 0; i < sign.input_signatures.size(); ++i)
        {
            std::vector<bool> boolVec;
            for (std::size_t j = 0; j < sign.input_signatures.size(); ++j)
            {
                bool value = (rand() % 2) == 0;
                for (std::size_t k = 0; k < sign.input_signatures[j].first; ++k)
                    boolVec.push_back(value);
            }

            for (const auto &po : cir1_po_partition.at(sign))
            {
                auto result = splitBySimType2(po, cones1, cir1_pi_partitions.at(po), boolVec, i);
                new_partition1[POSignature(cir1_pi_partitions.at(po))].insert(po);
                split = split || result;
            }
            for (const auto &po : cir2_po_partition.at(sign))
            {
                auto result = splitBySimType2(po, cones2, cir2_pi_partitions.at(po), boolVec, i);
                new_partition2[POSignature(cir2_pi_partitions.at(po))].insert(po);
                split = split || result;
            }
            if (split)
                break;
        }
    }
    cir1_po_partition = new_partition1;
    cir2_po_partition = new_partition2;
    return split;
}

void Matcher::splitBySupport(POPartition &po_partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones)
{
    auto partition_copy = po_partition;
    po_partition.clear();

    for (const auto &cluster : partition_copy)
    {
        std::map<std::size_t, IOSet> aux_map;
        for (const auto &po : cluster.second)
        {
            std::size_t support_size = cones.at(po)->getInputs().size();
            aux_map[support_size].insert(po);
        }
        for (const auto &new_cluster : aux_map)
        {
            POSignature new_sign = cluster.first;
            new_sign.support_size = new_cluster.first;
            new_sign.input_signatures = { {new_cluster.first, PISignature()} };
            po_partition.insert({new_sign, new_cluster.second});
        }
    }
}

void Matcher::splitByUnateness(POPartition &po_partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones)
{
    auto partition_copy = po_partition;
    po_partition.clear();

    for (const auto &cluster : partition_copy)
    {
        for (const auto &po : cluster.second)
        {
            auto input_unateness = Simulator(cones.at(po)).simulate(1000);
            PIPartition new_pi_partition;
            for (const auto& pi_cluster : pi_partitions.at(po))
            {
                std::map<Unateness, IOSet> unateness_map;
                for (const auto &pi : pi_cluster.second)
                {
                    const UnatenessSet &unat_set = input_unateness.at(pi);
                    if (unat_set.find(Unateness::Binate) != unat_set.end()) //maybe should use more strict checking
                        unateness_map[Unateness::Binate].insert(pi);
                    else if (unat_set.find(Unateness::PosUnate) != unat_set.end())
                        unateness_map[Unateness::PosUnate].insert(pi);
                    else if (unat_set.find(Unateness::NegUnate) != unat_set.end())
                        unateness_map[Unateness::NegUnate].insert(pi);
                    else
                        log("Error during splitting by unateness");
                }
                for (const auto &it : unateness_map)
                {
                    PISignature new_pi_sign = pi_cluster.first;
                    new_pi_sign.unat = it.first;
                    new_pi_partition.push_back({new_pi_sign, it.second});
                }
            }
            pi_partitions.at(po) = new_pi_partition;
            po_partition[POSignature(new_pi_partition)].insert(po);
        }
    }
}

void Matcher::splitBySymmetry(POPartition &po_partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones)
{
    auto partition_copy = po_partition;
    po_partition.clear();

    for (const auto &cluster : partition_copy)
    {
        for (const auto &po : cluster.second)
        {
            auto sym_partition = Simulator(cones.at(po)).simulateSym(1000);
            PIPartition new_pi_partition;
            for (const auto& pi_cluster : pi_partitions.at(po))
            {
                IOSet unviewed_inputs = pi_cluster.second;
                IOSet non_sym_inputs;
                for (const auto &pi : pi_cluster.second)
                {
                    if (unviewed_inputs.find(pi) == unviewed_inputs.end())
                        continue;

                    for (const auto &sym_cluster : sym_partition)
                    {
                        if (sym_cluster.second.find(pi) == sym_cluster.second.end())
                            continue;

                        if (sym_cluster.first == Symmetry::None)
                        {
                            non_sym_inputs.insert(pi);
                            unviewed_inputs.erase(pi);
                        }
                        else
                        {
                            for (const auto &sym_pi : sym_cluster.second)
                            {
                                if (unviewed_inputs.find(sym_pi) == unviewed_inputs.end())
                                    log("Error during splitting by symmetry");
                                unviewed_inputs.erase(sym_pi);
                            }
                            PISignature new_pi_sign = pi_cluster.first;
                            new_pi_sign.sym = sym_cluster.first;
                            new_pi_partition.push_back({new_pi_sign, sym_cluster.second});
                        }
                        break;
                    }
                }
                if (!non_sym_inputs.empty())
                {
                    PISignature non_sym_sign = pi_cluster.first;
                    non_sym_sign.sym = Symmetry::None;
                    new_pi_partition.push_back({non_sym_sign, non_sym_inputs});
                }
            }
            pi_partitions.at(po) = new_pi_partition;
            po_partition[POSignature(new_pi_partition)].insert(po);
        }
    }
}

bool Matcher::splitBySimType1(const std::string &po, const Cones &cones, PIPartition &pi_partition, const std::vector<bool> &base_vec, std::size_t pi_cluster_ind)
{
    auto partition_copy = pi_partition;
    pi_partition.clear();

    InVector in_vec;
    std::size_t j = 0;
    for (std::size_t i = 0; i < partition_copy.size(); ++i)
    {
        for (const auto &pi : partition_copy[i].second)
        {
            in_vec.insert({pi, base_vec[j]});
            ++j;
        }
    }

    bool split = false;
    for (std::size_t i = 0; i < partition_copy.size(); ++i)
    {
        if (i != pi_cluster_ind)
        {
            pi_partition.push_back(partition_copy[i]);
            continue;
        }

        IOSet set0, set1;
        for (const auto &pi : partition_copy[i].second)
        {
            InVector in_vec_flip = in_vec;
            in_vec_flip.at(pi) = !in_vec_flip.at(pi);
            if (cones.at(po)->evalOutput(po, in_vec_flip))
                set1.insert(pi);
            else
                set0.insert(pi);
        }
        if (!set0.empty() && !set1.empty())
        {
            split = true;

            PISignature new_sign0 = partition_copy[i].first;
            new_sign0.simType1.push_back(false);
            pi_partition.push_back({new_sign0, set0});

            PISignature new_sign1 = partition_copy[i].first;
            new_sign1.simType1.push_back(true);
            pi_partition.push_back({new_sign1, set1});
        }
        else
        {
            pi_partition.push_back(partition_copy[i]);
        }
    }

    return split;
}

bool Matcher::splitBySimType2(const std::string &po, const Cones &cones, PIPartition &pi_partition, const std::vector<bool> &base_vec, std::size_t pi_cluster_ind)
{
    auto partition_copy = pi_partition;
    pi_partition.clear();

    InVector in_vec;
    std::size_t j = 0;
    for (std::size_t i = 0; i < partition_copy.size(); ++i)
    {
        for (const auto &pi : partition_copy[i].second)
        {
            in_vec.insert({pi, base_vec[j]});
            ++j;
        }
    }

    bool split = false;
    for (std::size_t i = 0; i < partition_copy.size(); ++i)
    {
        if (i != pi_cluster_ind)
        {
            pi_partition.push_back(partition_copy[i]);
            continue;
        }

        std::map<int, IOSet> val_sets;
        for (const auto &pi1 : partition_copy[i].second)
        {
            InVector in_vec_flip = in_vec;
            in_vec_flip.at(pi1) = !in_vec_flip.at(pi1);
            int output_weight = 0;
            for (const auto &pi2 : partition_copy[i].second)
            {
                if (pi1 == pi2)
                    continue;

                InVector new_in_vec = in_vec_flip;
                new_in_vec.at(pi2) = !new_in_vec.at(pi2);
                if (cones.at(po)->evalOutput(po, new_in_vec))
                    ++output_weight;
            }
            val_sets[output_weight].insert(pi1);
        }

        if (val_sets.size() > 1)
        {
            split = true;

            for (const auto& set : val_sets)
            {
                PISignature new_sign = partition_copy[i].first;
                new_sign.simType2.push_back(set.first);
                pi_partition.push_back({new_sign, set.second});
            }
        }
        else
        {
            pi_partition.push_back(partition_copy[i]);
        }
    }

    return split;
}

std::pair<POPartition, POPartition> Matcher::getPOPartitions() const
{
    return std::make_pair(cir1_po_partition, cir2_po_partition);
}

POSignature::POSignature(Circuit *cir) :
    support_size(-1)
{
    input_signatures = { {cir->getInputs().size(), PISignature()} };
}

POSignature::POSignature(const PIPartition &pi_partition)
{
    support_size = 0;
    for (const auto &cluster : pi_partition)
    {
        std::size_t cluster_size = cluster.second.size();
        support_size += cluster_size;
        input_signatures.push_back({cluster_size, cluster.first});
    }
}

bool POSignature::operator <(const POSignature &rhs) const
{
    if (support_size != rhs.support_size)
        return support_size < rhs.support_size;

    if (input_signatures.size() != rhs.input_signatures.size())
        return input_signatures.size() < rhs.input_signatures.size();

    for (std::size_t i = 0; i < input_signatures.size(); ++i)
    {
        std::size_t cluster_size1 = input_signatures[i].first,
                    cluster_size2 = rhs.input_signatures[i].first;
        const PISignature &pi_sign1 = input_signatures[i].second,
                          &pi_sign2 = rhs.input_signatures[i].second;

        if (cluster_size1 != cluster_size2)
            return cluster_size1 < cluster_size2;
        if (pi_sign1.unat != pi_sign2.unat)
            return pi_sign1.unat < pi_sign2.unat;
        if (pi_sign1.sym != pi_sign2.sym)
            return pi_sign1.sym < pi_sign2.sym;

        if (pi_sign1.simType1.size() != pi_sign2.simType1.size())
            return pi_sign1.simType1.size() < pi_sign2.simType1.size();

        for (std::size_t j = 0; j < pi_sign1.simType1.size(); ++j)
        {
            if (pi_sign1.simType1[j] != pi_sign2.simType1[j])
                return !pi_sign1.simType1[j] && pi_sign2.simType1[j]; //ordering
        }

        if (pi_sign1.simType2.size() != pi_sign2.simType2.size())
            return pi_sign1.simType2.size() < pi_sign2.simType2.size();

        for (std::size_t j = 0; j < pi_sign1.simType2.size(); ++j)
        {
            if (pi_sign1.simType2[j] != pi_sign2.simType2[j])
                return pi_sign1.simType2[j] < pi_sign2.simType2[j]; //ordering
        }
    }

    return false;

//    if (input_clusters.size() != rhs.input_clusters.size())
//        return input_clusters.size() < rhs.input_clusters.size();

//    for (std::size_t i = 0; i < input_clusters.size(); ++i)
//    {
//        if (input_clusters[i].size() != rhs.input_clusters[i].size())
//            return input_clusters[i].size() < rhs.input_clusters[i].size();
//    }
//    return true;
}

PISignature::PISignature() :
    unat(Unateness::Unknown),
    sym(Symmetry::Unknown)
{}
