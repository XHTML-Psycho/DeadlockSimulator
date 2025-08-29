#include "circularwaithandler.h"
#include <Objects/ProcessHandler.h>

CircularWaitHandler::CircularWaitHandler()
{
}

CircularWaitHandler::~CircularWaitHandler()
{
}

QList<int> CircularWaitHandler::getNextResource(ManagedProcess process)
{
    // Determine the next resource and quantity to be allocated
    QList<int> result;

    // Copy of the neededResources list to find the correct index because the sorting is local
    // and not in the process list in the handler object
    QList<ManagedResource> copyNeededResources = process.getNeededResources();

    // Use the avoidance algorithm which sorts the neededResources list
    process.setNeededResources(checkForDeadlock(process.getNeededResources()));

    // Next resource identifier to be allocated
    // -1: Resource not yet determined or no resource available
    // -5: No resources remaining to process
    int nextResource = -1;

    // Quantity of the resource to be allocated
    int countResource = -1;

    // Index of the next resource in the original neededResources list
    int indexResourceList = -1;

    // Iterate through the sorted neededResources list to find the next allocatable resource
    for(int i = 0; i < process.getNeededResources().count(); i++){

        // Check if the requested resource count is within available limits and greater than zero
        if(process.getNeededResources().at(i).getCount() <= ProcessHandler::availableResources_E[process.getNeededResources().at(i).getResourceId()] && process.getNeededResources().at(i).getCount() > 0){

            // Resource found - set allocation parameters
            nextResource = process.getNeededResources().at(i).getResourceId();
            indexResourceList = copyNeededResources.indexOf(process.getNeededResources().at(i));
            countResource = process.getNeededResources().at(i).getCount();
            break;

        } else if(i == process.getNeededResources().count() - 1 && nextResource == -1){
            // No more resources to process (all are either satisfied or exceed available capacity)
            nextResource = -5;
            break;
        }
    }

    result.append(nextResource);
    result.append(countResource);
    result.append(indexResourceList);

    return result;
}

QList<ManagedResource> CircularWaitHandler::checkForDeadlock(QList<ManagedResource> neededResources)
{
    // Sort resources by ID to ensure consistent ordering across all processes
    std::sort(neededResources.begin(), neededResources.end(), [](const ManagedResource& a, const ManagedResource& b) {
        return a.getResourceId() < b.getResourceId();
    });
    return neededResources;
}
