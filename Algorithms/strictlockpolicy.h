#ifndef STRICTLOCKPOLICY_H
#define STRICTLOCKPOLICY_H

#include <Algorithms/resource_manager_api.h>
#include <QMutex>

/**
 * @brief Class implementing the NoPreemption policy.
 *
 * This algorithm enforces a strict lock policy to avoid preemption:
 * once a process acquires resources, they cannot be preempted until
 * the process voluntarily releases them or the reallocator revokes them
 * after exceeding its time slot.
 */
class StrictLockPolicy : public resource_manager_api
{
public:
    /**
     * @brief Standard constructor for StrictLockPolicy
     */
    StrictLockPolicy();

    /**
     * @brief Destructor for StrictLockPolicy
     */
    ~StrictLockPolicy();

    /**
     * @brief Determines the next resource allocation for a given process
     *
     * Under the NoPreemption policy, allocation is restricted by strict
     * slot locking and cannot be forcibly taken by other processes.
     *
     * @param process The managed process requesting resource allocation
     * @return QList<int> containing three values:
     *         [0] nextResource: Resource ID to allocate (-1: not found, -2: retry needed, -5: no resources left)
     *         [1] countResource: Quantity of the resource to allocate
     *         [2] indexResourceList: Index of the resource in the process's needed resources list
     */
    QList<int> getNextResource(ManagedProcess process) override;

    /**
     * @brief Callback when a process can acquire resources
     * @param processId Id of the process that can proceed
     */
    void onAcquireConditionMet(int processId) override;

    /**
     * @brief Check if a process is allowed to acquire resources
     * @param processId Id of the process
     * @return true if allocation is permitted, false otherwise
     */
    bool canAcquire(int processId) override;

public:
    // Slot states to track whether resources are locked
    static bool slotPrinterLocked;
    static bool slotCDLocked;
    static bool slotPlotterLocked;
    static bool slotTapeDriveLocked;

    // Flags to track last revoked process by ID
    static bool lastRevokedProcessA;
    static bool lastRevokedProcessB;
    static bool lastRevokedProcessC;

    // Mutex for guarding resource allocation operations
    static QMutex *mutex;
};

#endif // STRICTLOCKPOLICY_H
