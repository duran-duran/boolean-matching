#pragma once

#include "circuit.h"

using IOSet = std::set<std::string>;

struct Signature
{
    Signature();

    std::size_t support_size;
    std::vector<IOSet> input_clusters;

    bool operator < (const Signature &rhs) const;
};

using IOPartition = std::map<Signature, IOSet>;
using Cones = std::map<std::string, Circuit *>;

class Matcher
{
public:
    Matcher(Circuit *cir1, Circuit *cir2);
    Matcher &splitBySupportSize();

    std::pair<IOPartition, IOPartition> getPOPartitions() const;
private:
    Circuit *cir1, *cir2;
    IOPartition cir1_partition, cir2_partition;
    Cones cones1, cones2;

    void splitBySupportSize(IOPartition &partition, Circuit *cir, const Cones &cones);
};
