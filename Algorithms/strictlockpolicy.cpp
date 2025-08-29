#include "strictlockpolicy.h"
#include <Objects/ProcessHandler.h>

bool StrictLockPolicy::slotPrinterLocked = false;
bool StrictLockPolicy::slotCDLocked = false;
bool StrictLockPolicy::slotPlotterLocked = false;
bool StrictLockPolicy::slotTapeDriveLocked = false;
bool StrictLockPolicy::lastRevokedProcessA = false;
bool StrictLockPolicy::lastRevokedProcessB = false;
bool StrictLockPolicy::lastRevokedProcessC = false;
QMutex *StrictLockPolicy::mutex = new QMutex();

StrictLockPolicy::StrictLockPolicy()
{
}

StrictLockPolicy::~StrictLockPolicy()
{
}

QList<int> StrictLockPolicy::getNextResource(ManagedProcess process)
{
    mutex->lock();

    // Determine the next resource and its count to be allocated
    QList<int> result;

    // Next resource identifier:
    // -1 → no resource yet determined
    // -2 → resource temporarily unavailable / retry needed
    // -5 → no resources left for this process
    int nextResource = -1;

    // Quantity of the resource to be allocated
    int countResource = -1;

    // Index of the chosen resource in the process's needed resources list
    int indexResourceList = -1;

    // Iterate through the process's needed resources to find the next allocatable resource
    for (int i = 0; i < process.getNeededResources().count(); i++) {
        if (process.getNeededResources().at(i).getCount() <=
            ProcessHandler::availableResources_E[process.getNeededResources().at(i).getResourceId()]
            && process.getNeededResources().at(i).getCount() > 0) {

            // Resource valid → record id, index, and count
            nextResource = process.getNeededResources().at(i).getResourceId();
            indexResourceList = i;
            countResource = process.getNeededResources().at(i).getCount();
            break;

        } else if (i == process.getNeededResources().count() - 1 && process.getNeededResources().at(i).getCount() > 0) {
            // Retry needed if no resource was allocated until now, but demand exists
            nextResource = -2;
        } else if (i == process.getNeededResources().count() - 1 && nextResource == -1) {
            // No resource available at all → termination for this process
            nextResource = -5;
            break;
        }
    }

    // Respect slot locks: if a slot is locked, retry (-2)
    switch (nextResource) {
    case 0:
        if (slotPrinterLocked) nextResource = -2;
        break;
    case 1:
        if (slotCDLocked) nextResource = -2;
        break;
    case 2:
        if (slotPlotterLocked) nextResource = -2;
        break;
    case 3:
        if (slotTapeDriveLocked) nextResource = -2;
        break;
    }

    result.append(nextResource);
    result.append(countResource);
    result.append(indexResourceList);

    mutex->unlock();
    return result;
}

void StrictLockPolicy::onAcquireConditionMet(int processId)
{
    // Reset flags when a process successfully acquires resources
    if (processId == 0) {
        StrictLockPolicy::lastRevokedProcessA = false;
    } else if (processId == 1) {
        StrictLockPolicy::lastRevokedProcessB = false;
    } else if (processId == 2) {
        StrictLockPolicy::lastRevokedProcessC = false;
    }
}

bool StrictLockPolicy::canAcquire(int processId)
{
    // Only allow re-acquire if the process was revoked in the last attempt
    return (StrictLockPolicy::lastRevokedProcessA && processId == 0) ||
           (StrictLockPolicy::lastRevokedProcessB && processId == 1) ||
           (StrictLockPolicy::lastRevokedProcessC && processId == 2);
}
