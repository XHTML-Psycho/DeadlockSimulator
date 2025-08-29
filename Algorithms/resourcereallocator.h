#ifndef RESOURCEREALLOCATOR_H
#define RESOURCEREALLOCATOR_H

#include "qtimer.h"
#include <QObject>
#include <Objects/ManagedProcess.h>

/**
 * @brief The ResourceReallocator class is for the NoPreemption algorithm.
 * It provides a separate thread which can revoke resources from processes when their timeslot expires.
 */
class ResourceReallocator : public QObject
{
    Q_OBJECT
public:
    explicit ResourceReallocator(QObject *parent = 0);

public slots:
    /**
     * @brief Slot triggered when the reservation of a resource starts
     * @param processId ID of the process requesting the resource
     * @param nextResource The next needed resource ID
     * @param nextCount Quantity of the next needed resource
     */
    void reservationStarted(int processId, int nextResource, int nextCount);

    /**
     * @brief Slot triggered when the reservation of a resource finishes
     * @param processId ID of the process
     * @param nextResource The next needed resource ID
     * @param nextCount Quantity of the next needed resource
     * @param notProcessedYet True if some resources are still not processed
     */
    void reservationFinished(int processId, int nextResource, int nextCount, bool notProcessedYet);

    /**
     * @brief Initialize timers for all resources
     */
    void initTimers();

    /**
     * @brief Slots to revoke resources when their timeslot is exceeded
     */
    void revokePrinter();
    void revokeCD();
    void revokePlotter();
    void revokeTapeDrive();

    /**
     * @brief Getter methods for timer activity status
     */
    bool getTimerPrinterStatus(){ return timerPrinter->isActive(); }
    bool getTimerCDStatus(){ return timerCD->isActive(); }
    bool getTimerPlotterStatus(){ return timerPlotter->isActive(); }
    bool getTimerTapeDriveStatus(){ return timerTapeDrive->isActive(); }

signals:
    /**
     * @brief Signal emitted when a resource has been released
     * @param processID ID of the process from which the resource is revoked
     * @param resource ID of the released resource
     * @param count Quantity of the released resource
     * @param notProcessedYet True if there are still resources pending
     */
    void resourceReleased(int processID, int resource, int count, bool notProcessedYet);

private:
    QTimer *timerPrinter, *timerCD, *timerPlotter, *timerTapeDrive;
    int nextPrinterResource = -1, nextCDResource = -1, nextPlotterResource = -1, nextTapeDriveResource = -1;
    int nextPrinterCount = 0, nextCDCount = 0, nextPlotterCount = 0, nextTapeDriveCount = 0;
    int processPrinter, processCD, processPlotter, processTapeDrive;
};

#endif // RESOURCEREALLOCATOR_H
