#ifndef BANKERSALGORITHM_H
#define BANKERSALGORITHM_H

#include <Objects/managedprocess.h>
#include <Algorithms/resource_manager_api.h>

using namespace std;

/**
 * @brief Class implementing the Banker's Algorithm for deadlock prevention
 */
class BankersAlgorithm : public resource_manager_api
{
public:
    /**
     * @brief BankersAlgorithm standard constructor
     */
    BankersAlgorithm();

    /**
     * @brief BankersAlgorithm destructor
     */
    ~BankersAlgorithm();

    /**
     * @brief Determines the next resource allocation for a given process
     *
     * This method analyzes the process's needed resources and finds the next
     * resource that can be safely allocated without causing a deadlock.
     * Uses the Banker's Algorithm to ensure system remains in a safe state.
     *
     * @param process The managed process requesting resource allocation
     * @return QList<int> containing three values:
     *         [0] nextResource: Resource ID to allocate (-1: not found, -2: retry needed, -5: no resources left)
     *         [1] countResource: Quantity of the resource to allocate
     *         [2] indexResourceList: Index of the resource in the process's needed resources list
     */
    QList<int> getNextResource(ManagedProcess process) override;

    /**
     * @brief Checks if the current system state would result in a deadlock
     *
     * Implements the safety algorithm portion of the Banker's Algorithm.
     * Simulates process completion to determine if all processes can finish
     * with the current resource allocation state.
     *
     * @return true if the system is in a safe state (no deadlock), false if deadlock detected
     */
    bool checkForDeadlock();
};

#endif // BANKERSALGORITHM_H
