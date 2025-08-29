#ifndef HOLDANDWAITHANDLER_H
#define HOLDANDWAITHANDLER_H

#include <Algorithms/resource_manager_api.h>
#include <QSemaphore>

/**
 * @brief Class represents the algorithm to eliminate HoldAndWait
 */
class HoldAndWaitHandler : public resource_manager_api
{
public:
    /**
     * @brief HoldAndWaitHandler standard constructor
     */
    HoldAndWaitHandler();

    /**
     * @brief HoldAndWaitHandler destructor
     */
    ~HoldAndWaitHandler();

    /**
     * @brief Determines the next resource allocation for a given process
     *
     * This method analyzes the process's needed resources and finds the next
     * resource that should be allocated to prevent hold-and-wait conditions.
     *
     * @param process The managed process requesting resource allocation
     * @return QList<int> containing three values:
     *         [0] nextResource: Resource ID to allocate (-1: not found, -5: no resources left)
     *         [1] countResource: Quantity of the resource to allocate
     *         [2] indexResourceList: Index of the resource in the process's needed resources list
     */
    QList<int> getNextResource(ManagedProcess process) override;

    /**
     * @brief Deadlock avoidance algorithm that checks for potential deadlocks
     *
     * This algorithm analyzes the current system state to determine if
     * allocating resources to the given process would cause a deadlock.
     *
     * @param process The managed process which runs in the current thread
     * @param differenceResources_A Array with the current available resources
     * @return true if allocation is safe (no deadlock), false if deadlock would occur
     */
    bool checkForDeadlock(ManagedProcess process, int differenceResources_A[4]);

private:
    static QSemaphore *semaphore;
    static QList<int> currentProcess;
    static int copyDifference[4];
};

#endif // HOLDANDWAITHANDLER_H
