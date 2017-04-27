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

void Matcher::splitByUnateness(POPartition &partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones)
{
    auto partition_copy = partition;
    partition.clear();

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
            partition[POSignature(new_pi_partition)].insert(po);
        }
    }
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
