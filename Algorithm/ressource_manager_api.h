//
// Created by junio on 21.08.2025.
//

#ifndef DEADLOCKSIMULATOR_RESSOURCE_MANAGER_API_H
#define DEADLOCKSIMULATOR_RESSOURCE_MANAGER_API_H

#include "processor.h"
#include <QSemaphore>
#include <QDebug>

class ressource_manager_api {
public:
    ressource_manager_api();

    virtual ~ressource_manager_api() = 0;


    virtual  QList<int> getNextResource(Processor process);

    virtual  void onAcquireConditionMet(int ProcessID){

    }
    virtual  bool canAquire(int processID){
        return false;
    }
};


#endif //DEADLOCKSIMULATOR_RESSOURCE_MANAGER_API_H
