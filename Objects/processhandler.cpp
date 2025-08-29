#include "processhandler.h"

/**
 * @brief semaphorePrinter regulates how many printers can be used by threads
 * @brief semaphoreCD regulates how many CDs can be used by threads
 * @brief semaphorePlotter regulates how many plotters can be used by threads
 * @brief semaphoreTapeDrive regulates how many tape drives can be used by threads
 */
QSemaphore* ProcessHandler::semaphorePrinter;
QSemaphore* ProcessHandler::semaphoreCD;
QSemaphore* ProcessHandler::semaphorePlotter;
QSemaphore* ProcessHandler::semaphoreTapeDrive;

// Initializing the static matrices
int ProcessHandler::differenceResources_A[4];
int ProcessHandler::availableResources_E[4];
int ProcessHandler::assignedResources_C[3][4];
int ProcessHandler::stillNeededResources_R[3][4];

ProcessHandler::ProcessHandler(ManagedProcess process, int selectedAlgorithm, QObject *parent) :
    QObject(parent)
{
    this->selectedAlgorithm = selectedAlgorithm;

    // Copying the process belonging to the thread with all attributes
    this->process = ManagedProcess();
    this->process.setNeededResources(process.getNeededResources());
    this->process.setProcessId(process.getProcessId());
    this->process.setName(process.getName());
    this->process.setRevokedResourceId(process.getRevokedResourceId());

    for(int i = 0; i < 4; i++){
        // resourceID is needed because the stillNeededResources array is ordered by resources,
        // but the process neededResources list is in random order
        int resourceID  = process.getNeededResources().at(i).getResourceId();
        ProcessHandler::stillNeededResources_R[process.getProcessId()][resourceID] = process.getNeededResources().at(i).getCount();
    }

    // Initializing the semaphores
    semaphorePrinter   = new QSemaphore(availableResources_E[0]);
    semaphoreCD        = new QSemaphore(availableResources_E[1]);
    semaphorePlotter   = new QSemaphore(availableResources_E[2]);
    semaphoreTapeDrive = new QSemaphore(availableResources_E[3]);
}

void ProcessHandler::requestResource()
{
    // ID of the last resource (-1: none acquired yet, -5: all resources processed)
    int lastResource = -1;
    // Quantity of the last acquired resource (-1 means none yet)
    int lastCount = -1;
    // Index of the lastResource in the process's neededResources list
    int lastIndexResourceList = -1;

    // Initialize the resource allocation algorithm
    resource_manager_api *algorithm;
    switch(selectedAlgorithm){
        case 0:
            algorithm = new HoldAndWaitHandler();
            break;
        case 1:
            algorithm = new StrictLockPolicy();
            break;
        case 2:
            algorithm = new CircularWaitHandler();
            break;
        case 3:
            algorithm = new BankersAlgorithm();
            break;
        default:
            algorithm = new DirectAllocationSim();
            break;
    }

    int nextResource, countResource, indexResourceList;

    // Process resources until all are allocated or algorithm signals retry
    while(lastResource != -5 || nextResource == -2){

        if(QThread::currentThread()->isInterruptionRequested()){
            return;
        }

        // Call the appropriate algorithm to determine next resource
        QList<int> foundNextResource = algorithm->getNextResource(process);
        nextResource = foundNextResource.at(0);
        countResource = foundNextResource.at(1);
        indexResourceList = foundNextResource.at(2);

        // If algorithm allows acquisition after revocation
        if(algorithm->canAcquire(process.getProcessId())){
            refreshProcess(lastIndexResourceList, process.getNeededResources().at(lastIndexResourceList).getCount() + lastCount);
            lastCount = 0;
            algorithm->onAcquireConditionMet(process.getProcessId());
        }

        if(nextResource >= 0){
            emit startedAcquire(process.getProcessId(), nextResource, countResource);

            // Reserve the resource via semaphore
            switch (nextResource) {
            case 0: semaphorePrinter->acquire(countResource); break;
            case 1: semaphoreCD->acquire(countResource); break;
            case 2: semaphorePlotter->acquire(countResource); break;
            case 3: semaphoreTapeDrive->acquire(countResource); break;
            }

            // Update resource matrices
            differenceResources_A[nextResource] -= countResource;
            assignedResources_C[process.getProcessId()][nextResource] += countResource;
            refreshProcess(indexResourceList, process.getNeededResources().at(indexResourceList).getCount() - countResource);
            emit resourceReserved(process.getProcessId(), nextResource, countResource);
        }

        if(algorithm->canAcquire(process.getProcessId())){
            refreshProcess(lastIndexResourceList, process.getNeededResources().at(lastIndexResourceList).getCount() + lastCount);
            lastCount = 0;
            algorithm->onAcquireConditionMet(process.getProcessId());
        }

        // Release previously held resource
        if(lastResource != -1 && lastResource != -2 && nextResource != -2 && nextResource != -1){
            switch (lastResource) {
            case 0: semaphorePrinter->release(lastCount); break;
            case 1: semaphoreCD->release(lastCount); break;
            case 2: semaphorePlotter->release(lastCount); break;
            case 3: semaphoreTapeDrive->release(lastCount); break;
            }

            emit resourceReleased(process.getProcessId(), lastResource, lastCount, false);

            differenceResources_A[lastResource] += lastCount;
            assignedResources_C[process.getProcessId()][lastResource] -= lastCount;

            if(nextResource == -5){
                break;
            }
        }

        // Simulate resource usage
        if(countResource >= 0){
            QThread::sleep(2*countResource);
        } else {
            QThread::sleep(2);
        }

        if(nextResource != -1 && nextResource != -2){
            lastResource = nextResource;
            lastCount = countResource;
            lastIndexResourceList = indexResourceList;
        }
    }
    emit finishedResourceProcessing(process.getProcessId());
}

// Update the neededResources list by replacing the count of a resource
void ProcessHandler::refreshProcess(int nextResource, int countResource)
{
    QList<ManagedResource> resourcesCopy = process.getNeededResources();
    ManagedResource resourceCopy = process.getNeededResources().at(nextResource);
    resourceCopy.setCount(countResource);
    resourcesCopy.replace(nextResource, resourceCopy);
    process.setNeededResources(resourcesCopy);
}
