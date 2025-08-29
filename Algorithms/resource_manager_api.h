#ifndef RESOURCE_MANAGER_API_H
#define RESOURCE_MANAGER_API_H

#include <Objects/ManagedProcess.h>
#include <QSemaphore>
#include <QDebug>

/**
 * @brief Abstract base class providing an API for resource management algorithms
 */
class resource_manager_api
{
public:
    resource_manager_api();

    virtual ~resource_manager_api() = 0;

    /**
     * @brief Virtual method to determine the next resource allocation for a process.
     * Different algorithms will provide their own implementation.
     *
     * @param process The managed process requesting resource allocation
     * @return QList<int> result containing:
     *         [0] nextResource (ID: -1 = none available, -5 = no resources left, -2 = retry required)
     *         [1] nextResourceCount (quantity of the resource)
     *         [2] indexResourceList (index of resource in the process's neededResources list)
     */
    virtual QList<int> getNextResource(ManagedProcess process);

    /**
     * @brief Callback when a process can finally acquire needed resources
     * @param processId Id of the process that can acquire the resources
     */
    virtual void onAcquireConditionMet(int processId){
    }

    /**
     * @brief Check if a process is allowed to acquire resources
     * @param processId Id of the process
     * @return true if allowed, false otherwise
     */
    virtual bool canAcquire(int processId){
        return false;
    }
};

#endif // RESOURCE_MANAGER_API_H
