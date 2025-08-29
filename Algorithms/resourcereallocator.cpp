#include "resourcereallocator.h"
#include <Objects/ProcessHandler.h>

int maxWaitTime;

ResourceReallocator::ResourceReallocator(QObject *parent) :
    QObject{parent}
{
    // Initialize the maximum slot time for the resources based on the largest available pool
    maxWaitTime = max(
        max(ProcessHandler::differenceResources_A[0], ProcessHandler::differenceResources_A[1]),
        max(ProcessHandler::differenceResources_A[2], ProcessHandler::differenceResources_A[3])
    );
    maxWaitTime *= 2500;
}

void ResourceReallocator::reservationStarted(int processId, int nextResource, int nextCount){
    // When a reservation starts, start the corresponding timer and save variables for release
    switch(nextResource){
    case 0:
        if(!StrictLockPolicy::slotPrinterLocked){
            StrictLockPolicy::slotPrinterLocked = true;
            timerPrinter->start(maxWaitTime);
            processPrinter = processId;
            nextPrinterResource = nextResource;
            nextPrinterCount = nextCount;
        }
        break;
    case 1:
        if(!StrictLockPolicy::slotCDLocked){
            StrictLockPolicy::slotCDLocked = true;
            timerCD->start(maxWaitTime);
            processCD = processId;
            nextCDResource = nextResource;
            nextCDCount = nextCount;
        }
        break;
    case 2:
        if(!StrictLockPolicy::slotPlotterLocked){
            StrictLockPolicy::slotPlotterLocked = true;
            timerPlotter->start(maxWaitTime);
            processPlotter = processId;
            nextPlotterResource = nextResource;
            nextPlotterCount = nextCount;
        }
        break;
    case 3:
        if(!StrictLockPolicy::slotTapeDriveLocked){
            StrictLockPolicy::slotTapeDriveLocked = true;
            timerTapeDrive->start(maxWaitTime);
            processTapeDrive = processId;
            nextTapeDriveResource = nextResource;
            nextTapeDriveCount = nextCount;
        }
        break;
    }
}

void ResourceReallocator::reservationFinished(int processId, int nextResource, int nextCount, bool notProcessedYet){
    // When the reservation is finished without exceeding the slot time, stop timers and release locks
    if(!notProcessedYet){
        switch(nextResource){
        case 0:
            StrictLockPolicy::slotPrinterLocked = false;
            timerPrinter->stop();
            nextPrinterCount = -1;
            break;
        case 1:
            StrictLockPolicy::slotCDLocked = false;
            timerCD->stop();
            nextCDCount = -1;
            break;
        case 2:
            StrictLockPolicy::slotPlotterLocked = false;
            timerPlotter->stop();
            nextPlotterCount = -1;
            break;
        case 3:
            StrictLockPolicy::slotTapeDriveLocked = false;
            timerTapeDrive->stop();
            nextTapeDriveCount = -1;
            break;
        }
    }
}

void ResourceReallocator::initTimers(){
    // Initialize timer objects and set up slot connections
    timerPrinter  = new QTimer();
    timerCD = new QTimer();
    timerPlotter = new QTimer();
    timerTapeDrive = new QTimer();

    connect(timerPrinter, SIGNAL(timeout()), this, SLOT(revokePrinter()), Qt::QueuedConnection);
    connect(timerCD, SIGNAL(timeout()), this, SLOT(revokeCD()), Qt::QueuedConnection);
    connect(timerPlotter, SIGNAL(timeout()), this, SLOT(revokePlotter()), Qt::QueuedConnection);
    connect(timerTapeDrive, SIGNAL(timeout()), this, SLOT(revokeTapeDrive()), Qt::QueuedConnection);
}

void ResourceReallocator::revokePrinter(){
    // Printer timeslot exceeded - revoke resource and notify
    ProcessHandler::semaphorePrinter->release(nextPrinterCount);
    switch(processPrinter){
    case 0: StrictLockPolicy::lastRevokedProcessA = true; break;
    case 1: StrictLockPolicy::lastRevokedProcessB = true; break;
    case 2: StrictLockPolicy::lastRevokedProcessC = true; break;
    }
    StrictLockPolicy::slotPrinterLocked = false;
    ProcessHandler::differenceResources_A[nextPrinterResource] += nextPrinterCount;
    ProcessHandler::assignedResources_C[processPrinter][nextPrinterResource] -= nextPrinterCount;
    emit resourceReleased(processPrinter, nextPrinterResource, nextPrinterCount, true);
}

void ResourceReallocator::revokeCD(){
    // CD timeslot exceeded - revoke resource and notify
    ProcessHandler::semaphoreCD->release(nextCDCount);
    switch(processCD){
    case 0: StrictLockPolicy::lastRevokedProcessA = true; break;
    case 1: StrictLockPolicy::lastRevokedProcessB = true; break;
    case 2: StrictLockPolicy::lastRevokedProcessC = true; break;
    }
    StrictLockPolicy::slotCDLocked = false;
    ProcessHandler::differenceResources_A[nextCDResource] += nextCDCount;
    ProcessHandler::assignedResources_C[processCD][nextCDResource] -= nextCDCount;
    emit resourceReleased(processCD, nextCDResource, nextCDCount, true);
}

void ResourceReallocator::revokePlotter(){
    // Plotter timeslot exceeded - revoke resource and notify
    ProcessHandler::semaphorePlotter->release(nextPlotterCount);
    switch(processPlotter){
    case 0: StrictLockPolicy::lastRevokedProcessA = true; break;
    case 1: StrictLockPolicy::lastRevokedProcessB = true; break;
    case 2: StrictLockPolicy::lastRevokedProcessC = true; break;
    }
    StrictLockPolicy::slotPlotterLocked = false;
    ProcessHandler::differenceResources_A[nextPlotterResource] += nextPlotterCount;
    ProcessHandler::assignedResources_C[processPlotter][nextPlotterResource] -= nextPlotterCount;
    emit resourceReleased(processPlotter, nextPlotterResource, nextPlotterCount, true);
}

void ResourceReallocator::revokeTapeDrive(){
    // TapeDrive timeslot exceeded - revoke resource and notify
    ProcessHandler::semaphoreTapeDrive->release(nextTapeDriveCount);
    switch(processTapeDrive){
    case 0: StrictLockPolicy::lastRevokedProcessA = true; break;
    case 1: StrictLockPolicy::lastRevokedProcessB = true; break;
    case 2: StrictLockPolicy::lastRevokedProcessC = true; break;
    }
    StrictLockPolicy::slotTapeDriveLocked = false;
    ProcessHandler::differenceResources_A[nextTapeDriveResource] += nextTapeDriveCount;
    ProcessHandler::assignedResources_C[processTapeDrive][nextTapeDriveResource] -= nextTapeDriveCount;
    emit resourceReleased(processTapeDrive, nextTapeDriveResource, nextTapeDriveCount, true);
}
