#ifndef CIRCULARWAITHANDLER_H
#define CIRCULARWAITHANDLER_H

#include <Algorithms/resource_manager_api.h>

/**
 * @brief Class represents the algorithm to eliminate CircularWait
 */
class CircularWaitHandler : public resource_manager_api
{
public:
    /**
     * @brief CircularWaitHandler standard constructor
     */
    CircularWaitHandler();

    /**
     * @brief CircularWaitHandler destructor
     */
    ~CircularWaitHandler();

    /**
     * @brief Determines the next resource allocation for a given process
     *
     * This method analyzes the process's needed resources and finds the next
     * resource that should be allocated to prevent circular wait conditions.
     *
     * @param process The managed process requesting resource allocation
     * @return QList<int> containing three values:
     *         [0] nextResource: Resource ID to allocate (-1: not found, -5: no resources left)
     *         [1] countResource: Quantity of the resource to allocate
     *         [2] indexResourceList: Index of the resource in the process's needed resources list
     */
    QList<int> getNextResource(ManagedProcess process) override;

    /**
     * @brief Sorts the needed resources to avoid circular wait deadlock
     *
     * This algorithm reorders the resource list to ensure resources are
     * always requested in a consistent order across all processes.
     *
     * @param neededResources List of the managed resources the process needs to work correctly
     * @return Returns the sorted list of resources
     */
    QList<ManagedResource> checkForDeadlock(QList<ManagedResource> neededResources);
};

#endif // CIRCULARWAITHANDLER_H
