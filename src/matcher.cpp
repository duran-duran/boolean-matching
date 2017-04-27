#include "matcher.h"

Matcher::Matcher(Circuit *cir1, Circuit *cir2) :
    cir1(cir1), cir2(cir2)
{
    auto getCones = [](std::map<std::string, Circuit *> &cones, Circuit *cir)
    {
        for (const auto &po : cir->getOutputs())
            cones.insert({po, cir->getCone(po)});
    };

    getCones(cones1, cir1);
    getCones(cones2, cir2);

    cir1_partition = { {Signature(), IOSet(cir1->getOutputs().begin(), cir1->getOutputs().end())} };
    cir2_partition = { {Signature(), IOSet(cir2->getOutputs().begin(), cir2->getOutputs().end())} };
}

Matcher &Matcher::splitBySupportSize()
{
    splitBySupportSize(cir1_partition, cir1, cones1);
    splitBySupportSize(cir2_partition, cir2, cones2);
    return *this;
}

void Matcher::splitBySupportSize(IOPartition &partition, Circuit *cir, const Cones &cones)
{
    auto partition_copy = partition;
    partition.clear();

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
            Signature new_sign = cluster.first;
            new_sign.support_size = new_cluster.first;
            partition.insert({new_sign, new_cluster.second});
        }
    }
}

std::pair<IOPartition, IOPartition> Matcher::getPOPartitions() const
{
    return std::make_pair(cir1_partition, cir2_partition);
}

Signature::Signature() :
    support_size(-1)
{}

bool Signature::operator <(const Signature &rhs) const
{
    if (support_size != rhs.support_size)
        return support_size < rhs.support_size;

    if (input_clusters.size() != rhs.input_clusters.size())
        return input_clusters.size() < rhs.input_clusters.size();

    for (std::size_t i = 0; i < input_clusters.size(); ++i)
    {
        if (input_clusters[i].size() != rhs.input_clusters[i].size())
            return input_clusters[i].size() < rhs.input_clusters[i].size();
    }
    return true;
}
