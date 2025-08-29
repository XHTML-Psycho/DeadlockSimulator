#include "managedprocess.h"
#include <algorithm>
#include <QDebug>
#include <QRandomGenerator>

ManagedProcess::ManagedProcess(QString name,int processId)
{
    this->name = name;
    this->processId = processId;
    this->revokedResourceId = -1;
}
int ManagedProcess::safeBounded(int min, int max) {
    if (max <= min) {
        qWarning() << "Invalid random range:" << min << "-" << max << ". Using min value.";
        return min;
    }
    return QRandomGenerator::global()->bounded(min, max);
}
ManagedProcess::ManagedProcess(QString name,int processId, int maxPrinters, int maxCDs, int maxPlotters, int maxTapeDrives)
{
    this->name = name;
    this->processId = processId;
    this->revokedResourceId = -1;



    // Verwendung:
    int randomPrinterCount = safeBounded(1, maxPrinters);
    int randomCDCount = safeBounded(1, maxCDs);
    int randomPlotterCount = safeBounded(1, maxPlotters);
    int randomTapeDriveCount = safeBounded(1, maxTapeDrives);

    neededResources.append(ManagedResource("Printer", 0, randomPrinterCount));
    neededResources.append(ManagedResource("CD-ROM", 1, randomCDCount));
    neededResources.append(ManagedResource("Plotter", 2, randomPlotterCount));
    neededResources.append(ManagedResource("TapeDrive", 3, randomTapeDriveCount));

    qDebug() << "Process" << name << "resources:"
             << "Printers:" << randomPrinterCount
             << "CDs:" << randomCDCount
             << "Plotters:" << randomPlotterCount
             << "TapeDrives:" << randomTapeDriveCount;

    // Shuffles the needed resources inside the neededResources list
    shuffleNeededResources();
}
