#include "holdandwaithandler.h"
#include <QDebug>
#include <Objects/ProcessHandler.h>

QList<int> HoldAndWaitHandler::currentProcess;
QSemaphore* HoldAndWaitHandler::semaphore = new QSemaphore(1);
int HoldAndWaitHandler::copyDifference[4];

HoldAndWaitHandler::HoldAndWaitHandler()
{
}

HoldAndWaitHandler::~HoldAndWaitHandler()
{
}

QList<int> HoldAndWaitHandler::getNextResource(ManagedProcess process){

    QList<int> result;

    // Next resource identifier to be allocated
    // -1: Resource not yet determined or no resource available
    // -2: Request must be retried due to resource constraints
    // -5: No resources remaining to process
    int nextResource = -1;

    // Quantity of the resource to be allocated
    int countResource = -1;

    // Index of the next resource in the process's needed resources list
    int indexResourceList = -1;

    // If this is the first call, set up the copy of the differenceResources
    // because it has to be changed separately
    if(currentProcess.empty()){
        // Flag as not empty
        currentProcess.append(-1);
        for(int i = 0; i < 4; i++){
            copyDifference[i] = ProcessHandler::differenceResources_A[i];
        }
    }

    // Test if process hasn't requested anything yet
    if(!currentProcess.contains(process.getProcessId())){
        // Try to acquire a semaphore
        semaphore->acquire(1);

        // If everything is free we can continue to reserve
        if(checkForDeadlock(process, copyDifference)){
            // Add process to currentProcess to show that it has now requested its resources
            currentProcess.append(process.getProcessId());

            // Update copyDifference array with all the counts of the current process
            for(int i = 0; i < 4; i++){
                copyDifference[process.getNeededResources().at(i).getResourceId()] -= process.getNeededResources().at(i).getCount();
            }

            // If there are still free resources then another process can try to reserve some
            if(copyDifference[0] > 0 || copyDifference[1] > 0 || copyDifference[2] > 0 || copyDifference[3] > 0){
                semaphore->release(1);
            }

        } else {
            // Not enough free resources, return to retry at a later point
            semaphore->release(1);
            nextResource = -2;
            result.append(nextResource);
            result.append(countResource);
            result.append(indexResourceList);
            return result;
        }
    }

    // Iterate through the neededResources list to find the next allocatable resource
    for(int i = 0; i < process.getNeededResources().count(); i++){
        // Check if the requested resource count is within available limits and greater than zero
        if(process.getNeededResources().at(i).getCount() <= ProcessHandler::availableResources_E[process.getNeededResources().at(i).getResourceId()] && process.getNeededResources().at(i).getCount() > 0){
            // Resource found - set allocation parameters
            nextResource = process.getNeededResources().at(i).getResourceId();
            indexResourceList = i;
            countResource = process.getNeededResources().at(i).getCount();
            // Update copyDifference
            copyDifference[nextResource] += countResource;
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

    // If there are no resources left to reserve, a semaphore will be released
    if(nextResource == -5){
       qDebug() << "unlocked";
       semaphore->release(1);
    }

    return result;
}

bool HoldAndWaitHandler::checkForDeadlock(ManagedProcess process, int copyDifference[4])
{
    bool resourcesAvailable = true;

    // Check if all needed resources are available
    for(int i = 0; i < process.getNeededResources().count(); i++){
        if(process.getNeededResources().at(i).getCount() > copyDifference[process.getNeededResources().at(i).getResourceId()]){
            resourcesAvailable = false;
        }
    }

    return resourcesAvailable;
}
