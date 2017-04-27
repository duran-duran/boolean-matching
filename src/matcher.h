#pragma once

#include "circuit.h"
#include "simulator.h"

using IOSet = std::set<std::string>;

struct PISignature
{
    PISignature();

    Unateness unat;
    Symmetry sym;
    std::vector<bool> simType1;
};

using PISignMask = std::vector<std::pair<std::size_t, PISignature>>;
using PIPartition = std::vector<std::pair<PISignature, IOSet>>;

struct POSignature
{
    POSignature(Circuit *cir);
    POSignature(const PIPartition &pi_partition);

    std::size_t support_size;
    PISignMask input_signatures;

    bool operator < (const POSignature &rhs) const;
};

using POPartition = std::map<POSignature, IOSet>;

using Cones = std::map<std::string, Circuit *>;

class Matcher
{
public:
    Matcher(Circuit *cir1, Circuit *cir2);
    ~Matcher();

    Matcher &splitBySupport();
    Matcher &splitByUnateness();
    Matcher &splitBySymmetry();
    Matcher &splitBySimType1(std::size_t max_it);

    std::pair<POPartition, POPartition> getPOPartitions() const;
private:
    Circuit *cir1, *cir2;
    POPartition cir1_po_partition, cir2_po_partition;
    std::map<std::string, PIPartition> cir1_pi_partitions, cir2_pi_partitions;
    Cones cones1, cones2;

    void splitBySupport(POPartition &po_partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones);
    void splitByUnateness(POPartition &po_partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones);
    void splitBySymmetry(POPartition &po_partition, std::map<std::string, PIPartition> &pi_partitions, Circuit *cir, const Cones &cones);

    bool splitBySimType1();
    bool splitBySimType1(const std::string &po, const Cones &cones, PIPartition &pi_partition, const std::vector<bool> &base_vec, std::size_t pi_cluster_ind);
};
