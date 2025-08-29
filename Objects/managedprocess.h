#ifndef MANAGEDPROCESS_H
#define MANAGEDPROCESS_H

#include <QList>
#include "Objects/managedresource.h"
#include <QDebug>
#include <random>
#include <QtGlobal>
#include <QVector>
#include <QRandomGenerator>

/**
 * @brief Class represents the processes which use the resources
 */
class ManagedProcess
{

private:
    /**
     * @brief name of the process
     * @brief processId (corresponding to name: A = id 1...)
     * @brief neededResources resources the process needs to run
     */
    QString name;
    int processId;
    QList<ManagedResource> neededResources;
    int revokedResourceId;

public:
    /**
     * @brief ManagedProcess standard constructor with no parameter
     */
    ManagedProcess(){

    };

    /**
     * @brief ManagedProcess constructor setting name and processID
     * @param name of a process
     * @param processId (corresponding to name: A = id 1...)
     */
    ManagedProcess(QString name,int processId);

    /**
     * @brief ManagedProcess constructor setting name and processId and filling neededResources with a resource of each type with a count between min and max
     * @param name of a process
     * @param processId (corresponding to name: A = id 1...)
     * @param maxPrinters the maximum count of printers the process can need
     * @param maxCDs the maximum count of CDs the process can need
     * @param maxPlotters the maximum count of plotters the process can need
     * @param maxTapeDrives the maximum count of tape drives the process can need
     */
    ManagedProcess(QString name,int processId, int maxPrinters, int maxCDs, int maxPlotters, int maxTapeDrives);

    /**
     * @brief getProcessId getter for processId
     * @return processId
     */
    int getProcessId() const{
        return processId;
    }

    /**
     * @brief setProcessId setter for processId
     * @param processId
     */
    void setProcessId(int processId){
        this->processId = processId;
    }

    /**
     * @brief getName getter for name
     * @return name
     */
    QString getName() const{
        return name;
    }

    /**
     * @brief setName setter for name
     * @param name
     */
    void setName(QString name){
        this->name = name;
    }

    int safeBounded(int min, int max);

    /**
     * @brief getRevokedResourceId getter for revokedResourceId
     * @return revokedResourceId
     */
    int getRevokedResourceId() const{
        return revokedResourceId;
    }

    /**
     * @brief setRevokedResourceId setter for revokedResourceId
     * @param revokedResourceId
     */
    void setRevokedResourceId(int revokedResourceId){
        this->revokedResourceId = revokedResourceId;
    }

    /**
     * @brief getNeededResources list of needed resources
     * @return neededResources
     */
    const QList<ManagedResource> getNeededResources() const{
        return neededResources;
    }

    /**
     * @brief setNeededResources setting the neededResources list
     * @param neededResources
     */
    void setNeededResources(QList<ManagedResource> neededResources) {
        ManagedProcess::neededResources = neededResources;
    }

    /**
     * @brief moveNeededResourceToBack the needed resource will be placed at the end of the list
     * @param index placed at index
     */
    void moveNeededResourceToBack(int index){
        neededResources.swapItemsAt(index, neededResources.count()-1);
    }

    /**
     * @brief shuffleNeededResources randomly shuffles the order of needed resources
     */
    void shuffleNeededResources(){
        int n = neededResources.size();
        QRandomGenerator rng = QRandomGenerator::securelySeeded();

        for (int i = n - 1; i > 0; --i) {
            int j = rng.bounded(i + 1); // Generate a random index in [0, i]
            std::swap(neededResources[i], neededResources[j]); // Swap the elements at indices i and j
        }
    }

    /**
     * @brief printNeededResources prints the needed resources for debugging
     */
    void printNeededResources(){
        QDebug dbg(QtDebugMsg);
        dbg << "Process " << name << ":" << "\n";
        for(int i = 0; i < neededResources.size(); i++){
            dbg << neededResources.at(i).getName() << " (" << neededResources.at(i).getCount() << "), ";
        }
    }
};

#endif // MANAGEDPROCESS_H
