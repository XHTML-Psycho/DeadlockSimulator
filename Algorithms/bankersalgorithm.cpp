#include "bankersalgorithm.h"
#include "QDebug"
#include "qmutex.h"
#include <QThread>
#include <Objects/ProcessHandler.h>

QMutex *mutexOne = new QMutex();

BankersAlgorithm::BankersAlgorithm()
{
}

BankersAlgorithm::~BankersAlgorithm()
{
}

QList<int> BankersAlgorithm::getNextResource(ManagedProcess process)
{
    mutexOne->lock();

    // Determine the next resource and quantity to be allocated
    QList<int> result;

    // Next resource identifier to be allocated. Values:
    // -1: Resource not yet determined or no resource available
    // -2: Request must be retried due to potential deadlock
    // -5: No resources remaining to process
    int nextResource = -1;

    // Quantity of the resource to be allocated
    int countResource = -1;

    // Index of the next resource in the process's needed resources list
    int indexResourceList = -1;

    // Flag indicating whether a deadlock was detected in this iteration
    bool deadlock = false;

    // Iterate through the process's needed resources to find the next allocatable resource
    for(int i = 0; i < process.getNeededResources().count(); i++){

        // Check if the requested resource count is available and greater than zero
        if(process.getNeededResources().at(i).getCount() <= ProcessHandler::differenceResources_A[process.getNeededResources().at(i).getResourceId()] && process.getNeededResources().at(i).getCount() > 0){

            // Temporarily allocate resources to test for safe state
            ProcessHandler::differenceResources_A[process.getNeededResources().at(i).getResourceId()] -= process.getNeededResources().at(i).getCount();
            ProcessHandler::stillNeededResources_R[process.getProcessId()][process.getNeededResources().at(i).getResourceId()] -= process.getNeededResources().at(i).getCount();

            if(checkForDeadlock()){
                // Safe state confirmed - set allocation parameters

                // Restore available resources (will be modified during actual allocation)
                ProcessHandler::differenceResources_A[process.getNeededResources().at(i).getResourceId()] += process.getNeededResources().at(i).getCount();
                nextResource = process.getNeededResources().at(i).getResourceId();
                indexResourceList = i;
                countResource = process.getNeededResources().at(i).getCount();
                deadlock = false;
                break;
            } else{
                // Unsafe state detected - reject allocation to prevent deadlock

                deadlock = true;
                nextResource = -2; // Indicates deadlock condition
                ProcessHandler::stillNeededResources_R[process.getProcessId()][process.getNeededResources().at(i).getResourceId()] += process.getNeededResources().at(i).getCount();

                // Restore available resources to previous state
                ProcessHandler::differenceResources_A[process.getNeededResources().at(i).getResourceId()] += process.getNeededResources().at(i).getCount();
            }

        } else if(process.getNeededResources().at(i).getCount() > 0 && process.getNeededResources().at(i).getCount()){
            // Required resources are currently unavailable (allocated to other processes)
            nextResource = -2;

        } else if(!deadlock && i == process.getNeededResources().count() - 1 && nextResource == -1){
            // No more resources to process (all are either satisfied or exceed available capacity)
            nextResource = -5;
            break;
        }
    }

    result.append(nextResource);
    result.append(countResource);
    result.append(indexResourceList);

    mutexOne->unlock();
    return result;
}

bool BankersAlgorithm::checkForDeadlock(){
    // Counter for processes that have completed analysis
    int countFinished = 0;

    // Vector containing IDs of processes that have completed analysis
    vector<int> markedFinished;

    while(countFinished < 3) {
        bool found = false;

        // Examine each process for completion possibility
        for(int i = 0; i < 3; i++){
            // Skip processes that have already been analyzed
            if(!binary_search(markedFinished.begin(), markedFinished.end(), i)) {
                int count = 0;

                // Check if all resource requirements can be satisfied
                for(int j = 0; j < 4; j++) {
                    if(ProcessHandler::stillNeededResources_R[i][j] <= ProcessHandler::differenceResources_A[j]) {
                        count++;
                        if(count == 4) {
                            // All resources can be satisfied - mark process as completable
                            markedFinished.push_back(i);  // Fixed: should be 'i' not 'j'
                            found = true;
                            countFinished++;
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        if(!found){
            return false; // Deadlock detected - no process can complete
        }
    }
    return true; // Safe state - no deadlock
}
