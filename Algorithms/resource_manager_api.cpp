#include "resource_manager_api.h"
#include <Objects/ProcessHandler.h>

resource_manager_api::resource_manager_api()
{
}

resource_manager_api::~resource_manager_api()
{
}

QList<int> resource_manager_api::getNextResource(ManagedProcess process)
{
    // Determine the next resource and quantity to be allocated
    QList<int> result;

    // Next resource identifier to be allocated
    // -1: Resource not yet determined or no resource available
    // -5: No resources remaining to process
    int nextResource = -1;

    // Quantity of the resource to be allocated
    int countResource = -1;

    // Index of the next resource in the process's needed resources list
    int indexResourceList = -1;

    // Iterate through the process's needed resources to find the next allocatable resource
    for(int i = 0; i < process.getNeededResources().count(); i++){

        // Check if the requested resource count is within available limits and greater than zero
        if(process.getNeededResources().at(i).getCount() <= ProcessHandler::availableResources_E[process.getNeededResources().at(i).getResourceId()] && process.getNeededResources().at(i).getCount() > 0){

            // Resource found - set allocation parameters
            nextResource = process.getNeededResources().at(i).getResourceId();
            indexResourceList = i;
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
