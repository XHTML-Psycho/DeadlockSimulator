#ifndef DIRECTALLOCATIONSIM_H
#define DIRECTALLOCATIONSIM_H

#include <Algorithms/resource_manager_api.h>

/**
 * @brief Class represents the standard algorithm to sort and use the resources, normally leads to a deadlock
 */
class DirectAllocationSim : public resource_manager_api
{
public:
    /**
     * @brief NoAvoidanceSimulation standard constructor
     */
    DirectAllocationSim();
    ~DirectAllocationSim();
};

#endif // DIRECTALLOCATIONSIM_H
