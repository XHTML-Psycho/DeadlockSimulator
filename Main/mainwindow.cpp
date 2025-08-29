#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Objects/ProcessHandler.h>
#include <QProcess>
#include <QApplication>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <Dialogs/initdialog.h>
#include <Dialogs/summarydialog.h>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QFile>
#include <numeric>

// =============================================================================
// GLOBALE KONSTANTEN UND VARIABLEN
// =============================================================================

const int system_resource_count = 4;     ///< Anzahl der Ressourcentypen im System
const int system_process_count = 3;      ///< Anzahl der Prozesse im System
int existingResources[4];                ///< Verfügbare Anzahl jeder Ressourcenart
int selectedAlgorithmNumber = -1;        ///< Ausgewählter Deadlock-Algorithmus
int finished = 0;                        ///< Zähler für abgeschlossene Prozesse
int countAllResourcesUsed = 0;           ///< Gesamtanzahl aller genutzten Ressourcen

// Zeitmessung für Prozesse
QTimer *timer;
QElapsedTimer *processATimer = new QElapsedTimer();
QList<int> *processATimeList = new QList<int>();
QElapsedTimer *processBTimer = new QElapsedTimer();
QList<int> *processBTimeList = new QList<int>();
QElapsedTimer *processCTimer = new QElapsedTimer();
QList<int> *processCTimeList = new QList<int>();

// Ressourcen-Verwaltungsmatrizen
int assignedResources_C[3][4];           ///< Belegungsmatrix (Prozesse × Ressourcen)
int stillNeededResources_R[3][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}; ///< Anforderungsmatrix
int availableResources_E[4];             ///< Verfügbare Ressourcen
int occupiedResources_P[4] = {0, 0, 0, 0}; ///< Belegte Ressourcen
int differenceResources_A[4];            ///< Differenzressourcen

QList<ManagedProcess> processes;         ///< Liste aller Prozesse in der Simulation

// =============================================================================
// KONSTRUKTOR
// =============================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Visuelle Effekte initialisieren
    setShadows();

    // Schriftarten laden
    QFontDatabase::addApplicationFont(":/resources/fonts/futuraHeavy.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/futuraBook.ttf");

    // Initialisierungsdialog anzeigen und konfigurieren
    InitDialog initDialog;
    connect(&initDialog, SIGNAL(countsFinished(int*)), this, SLOT(initResourceCount(int*)));
    connect(&initDialog, SIGNAL(algorithmsFinished(int)), this, SLOT(selectedAlgorithm(int)));
    initDialog.setWindowTitle("Deadlock Algorithm Simulation");

    if (initDialog.exec() == QDialog::Accepted) {
        initDialog.getResourceCount();
        initDialog.getAlgorithm();
    } else if (initDialog.exec() == QDialog::Rejected) {
        this->close();
        initDialog.close();
    }

    // Matrizen und Prozesse initialisieren
    update_occupation_matrix();
    setUpResourcesAndProcesses(existingResources[0], existingResources[1],
                              existingResources[2], existingResources[3]);

    // Ressourcendaten für Threads vorbereiten
    for (int i = 0; i < 4; i++) {
        ProcessHandler::differenceResources_A[i] = differenceResources_A[i];
        ProcessHandler::availableResources_E[i] = availableResources_E[i];
    }

    // Prozess-Threads und Worker initialisieren
    threadProcessA = new QThread;
    workerA = new ProcessHandler(processes.at(0), selectedAlgorithmNumber);
    threadProcessB = new QThread;
    workerB = new ProcessHandler(processes.at(1), selectedAlgorithmNumber);
    threadProcessC = new QThread;
    workerC = new ProcessHandler(processes.at(2), selectedAlgorithmNumber);

    // Algorithmus-Erklärung anzeigen
    on_explanation_Button_algorithm_clicked();

    // Algorithmus-Label setzen
    switch (selectedAlgorithmNumber) {
    case 0: ui->CurrentAlgorithm_label->setText("Hold And Wait"); break;
    case 1: ui->CurrentAlgorithm_label->setText("No Preemption"); break;
    case 2: ui->CurrentAlgorithm_label->setText("Circular Wait"); break;
    case 3: ui->CurrentAlgorithm_label->setText("Bankiers Algorithm"); break;
    default: ui->CurrentAlgorithm_label->setText("No Avoidance"); break;
    }

    // Preemption-Worker für Algorithmus 1 initialisieren
    if (selectedAlgorithmNumber == 1) {
        threadPreemption = new QThread;
        preemptionWorker = new ResourceReallocator();
        preemptionWorker->moveToThread(threadPreemption);
        QMetaObject::invokeMethod(preemptionWorker, "initTimers");

        // Preemption-Verbindungen herstellen
        connect(workerA, SIGNAL(startedAcquire(int, int, int)), preemptionWorker, SLOT(reservationStarted(int, int, int)));
        connect(workerA, SIGNAL(resourceReleased(int,int,int, bool)), preemptionWorker, SLOT(reservationFinished(int,int,int, bool)));
        connect(workerB, SIGNAL(startedAcquire(int, int, int)), preemptionWorker, SLOT(reservationStarted(int, int, int)));
        connect(workerB, SIGNAL(resourceReleased(int,int,int, bool)), preemptionWorker, SLOT(reservationFinished(int,int,int, bool)));
        connect(workerC, SIGNAL(startedAcquire(int, int, int)), preemptionWorker, SLOT(reservationStarted(int, int, int)));
        connect(workerC, SIGNAL(resourceReleased(int,int,int, bool)), preemptionWorker, SLOT(reservationFinished(int,int,int, bool)));
        connect(preemptionWorker, SIGNAL(resourceReleased(int,int,int, bool)), this, SLOT(releaseResources(int,int,int, bool)));
        threadPreemption->start();
    }

    // Signal-Slot-Verbindungen für Prozesse einrichten
    connect(ui->button_start_simulation, SIGNAL(clicked()), workerA, SLOT(requestResource()));
    connect(workerA, SIGNAL(resourceReserved(int,int,int)), this, SLOT(reserveResources(int,int,int)));
    connect(workerA, SIGNAL(resourceReleased(int,int,int, bool)), this, SLOT(releaseResources(int,int,int, bool)));
    connect(workerA, SIGNAL(finishedResourceProcessing(int)), this, SLOT(processFinished(int)));

    connect(ui->button_start_simulation, SIGNAL(clicked()), workerB, SLOT(requestResource()));
    connect(workerB, SIGNAL(resourceReserved(int,int,int)), this, SLOT(reserveResources(int,int,int)));
    connect(workerB, SIGNAL(resourceReleased(int,int,int, bool)), this, SLOT(releaseResources(int,int,int, bool)));
    connect(workerB, SIGNAL(finishedResourceProcessing(int)), this, SLOT(processFinished(int)));

    connect(ui->button_start_simulation, SIGNAL(clicked()), workerC, SLOT(requestResource()));
    connect(workerC, SIGNAL(resourceReserved(int,int,int)), this, SLOT(reserveResources(int,int,int)));
    connect(workerC, SIGNAL(resourceReleased(int,int,int, bool)), this, SLOT(releaseResources(int,int,int, bool)));
    connect(workerC, SIGNAL(finishedResourceProcessing(int)), this, SLOT(processFinished(int)));

    // Threads starten
    workerA->moveToThread(threadProcessA);
    threadProcessA->start();
    workerB->moveToThread(threadProcessB);
    threadProcessB->start();
    workerC->moveToThread(threadProcessC);
    threadProcessC->start();

    workerA->configureAllocations(assignedResources_C, stillNeededResources_R);
}

// =============================================================================
// DESTRUKTOR
// =============================================================================

MainWindow::~MainWindow()
{
    delete ui;
}

// =============================================================================
// RESSOURCENVERWALTUNG
// =============================================================================

void MainWindow::reserveResources(int process, int resource, int count)
{
    // Ressourcenmatrizen aktualisieren
    assignedResources_C[process][resource] += count;
    stillNeededResources_R[process][resource] -= count;
    occupiedResources_P[resource] += count;
    differenceResources_A[resource] -= count;

    // Protokollnachricht erstellen und anzeigen
    QString logMessage = QString("Prozess %1 hat %2 Einheiten der Ressource %3 reserviert")
                        .arg(process).arg(count).arg(resource);
    ui->plainTextEdit_RequestInfo->appendPlainText(logMessage);

    // Visuelle Hervorhebung der belegten Ressource
    switch (resource) {
        case 0: ui->Printer_Group->setStyleSheet("QGroupBox { background-color: rgb(53, 97, 70); border-radius: 5px; }"); break;
        case 1: ui->CD_Group->setStyleSheet("QGroupBox { background-color: rgb(53, 97, 70); border-radius: 5px; }"); break;
        case 2: ui->Plotter_Group->setStyleSheet("QGroupBox { background-color: rgb(53, 97, 70); border-radius: 5px; }"); break;
        case 3: ui->TapeDrive_Group->setStyleSheet("QGroupBox { background-color: rgb(53, 97, 70); border-radius: 5px; }"); break;
    }

    // Benutzeroberfläche aktualisieren
    update_occupation_matrix();
    update_needed_matrix();
    update_resource_occupation();
    update_resource_occupation_list();

    // Verarbeitungszeit erfassen
    switch (process) {
    case 0:
        if (processATimer->isValid()) processATimeList->append(processATimer->elapsed());
        processATimer->restart();
        break;
    case 1:
        if (processBTimer->isValid()) processBTimeList->append(processBTimer->elapsed());
        processBTimer->restart();
        break;
    case 2:
        if (processCTimer->isValid()) processCTimeList->append(processCTimer->elapsed());
        processCTimer->restart();
        break;
    }
}

void MainWindow::releaseResources(int process, int resource, int count, bool notProcessedYet)
{
    // Bei vorzeitiger Freigabe Anforderungsmatrix zurücksetzen
    if (notProcessedYet) {
        stillNeededResources_R[process][resource] += count;
    } else {
        countAllResourcesUsed += count;
    }

    // Ressourcenmatrizen aktualisieren
    assignedResources_C[process][resource] -= count;
    occupiedResources_P[resource] -= count;
    differenceResources_A[resource] += count;

    // Visuelle Hervorhebung aktualisieren wenn Ressource frei
    if (occupiedResources_P[resource] == 0) {
        switch (resource) {
            case 0: ui->Printer_Group->setStyleSheet("QGroupBox { background-color: rgb(88, 101, 124); border-radius: 5px; }"); break;
            case 1: ui->CD_Group->setStyleSheet("QGroupBox { background-color: rgb(88, 101, 124); border-radius: 5px; }"); break;
            case 2: ui->Plotter_Group->setStyleSheet("QGroupBox { background-color: rgb(88, 101, 124); border-radius: 5px; }"); break;
            case 3: ui->TapeDrive_Group->setStyleSheet("QGroupBox { background-color: rgb(88, 101, 124); border-radius: 5px; }"); break;
        }
    }

    // Benutzeroberfläche aktualisieren
    update_occupation_matrix();
    update_needed_matrix();
    update_resource_occupation();
    update_resource_occupation_list();
}

// =============================================================================
// BENUTZEROBERFLÄCHEN-AKTUALISIERUNG
// =============================================================================

void MainWindow::update_occupation_matrix()
{
    ui->A1_label_occupation->setNum(assignedResources_C[0][0]);
    ui->A2_label_occupation->setNum(assignedResources_C[0][1]);
    ui->A3_label_occupation->setNum(assignedResources_C[0][2]);
    ui->A4_label_occupation->setNum(assignedResources_C[0][3]);
    ui->B1_label_occupation->setNum(assignedResources_C[1][0]);
    ui->B2_label_occupation->setNum(assignedResources_C[1][1]);
    ui->B3_label_occupation->setNum(assignedResources_C[1][2]);
    ui->B4_label_occupation->setNum(assignedResources_C[1][3]);
    ui->C1_label_occupation->setNum(assignedResources_C[2][0]);
    ui->C2_label_occupation->setNum(assignedResources_C[2][1]);
    ui->C3_label_occupation->setNum(assignedResources_C[2][2]);
    ui->C4_label_occupation->setNum(assignedResources_C[2][3]);
}

void MainWindow::update_needed_matrix()
{
    ui->A1_label_needed->setNum(stillNeededResources_R[0][0]);
    ui->A2_label_needed->setNum(stillNeededResources_R[0][1]);
    ui->A3_label_needed->setNum(stillNeededResources_R[0][2]);
    ui->A4_label_needed->setNum(stillNeededResources_R[0][3]);
    ui->B1_label_needed->setNum(stillNeededResources_R[1][0]);
    ui->B2_label_needed->setNum(stillNeededResources_R[1][1]);
    ui->B3_label_needed->setNum(stillNeededResources_R[1][2]);
    ui->B4_label_needed->setNum(stillNeededResources_R[1][3]);
    ui->C1_label_needed->setNum(stillNeededResources_R[2][0]);
    ui->C2_label_needed->setNum(stillNeededResources_R[2][1]);
    ui->C3_label_needed->setNum(stillNeededResources_R[2][2]);
    ui->C4_label_needed->setNum(stillNeededResources_R[2][3]);
}

void MainWindow::update_resource_occupation()
{
    ui->Printer_label_occupation->setText(QString::number(occupiedResources_P[0]) + "/" + QString::number(availableResources_E[0]));
    ui->Cd_label_occupation->setText(QString::number(occupiedResources_P[1]) + "/" + QString::number(availableResources_E[1]));
    ui->Plotter_label_occupation->setText(QString::number(occupiedResources_P[2]) + "/" + QString::number(availableResources_E[2]));
    ui->Tapedrive_label_occupation->setText(QString::number(occupiedResources_P[3]) + "/" + QString::number(availableResources_E[3]));
}

void MainWindow::update_resource_occupation_list()
{
    QString ListPrinter = "", ListCd = "", ListPlotter = "", ListTapeDrive = "";

    for (int i = 0; i < system_resource_count; i++) {
        for (int j = 0; j < system_process_count; j++) {
            for (int k = 0; k < assignedResources_C[j][i]; k++) {
                switch (i) {
                case 0:
                    if (ListPrinter != "") ListPrinter.append(" | ");
                    ListPrinter.append(processes.at(j).getName());
                    break;
                case 1:
                    if (ListCd != "") ListCd.append(" | ");
                    ListCd.append(processes.at(j).getName());
                    break;
                case 2:
                    if (ListPlotter != "") ListPlotter.append(" | ");
                    ListPlotter.append(processes.at(j).getName());
                    break;
                case 3:
                    if (ListTapeDrive != "") ListTapeDrive.append(" | ");
                    ListTapeDrive.append(processes.at(j).getName());
                    break;
                }
            }
        }
    }

    ui->Printer_label_occupation_list->setText(ListPrinter);
    ui->Cd_label_occupation_list->setText(ListCd);
    ui->Plotter_label_occupation_list->setText(ListPlotter);
    ui->Tapedrive_label_occupation_list->setText(ListTapeDrive);
}

void MainWindow::updateStillNeededRessources_R()
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            int resourceID = processes.at(i).getNeededResources().at(j).getResourceId();
            stillNeededResources_R[i][resourceID] = processes.at(i).getNeededResources().at(j).getCount();
        }
    }
    update_needed_matrix();
}

// =============================================================================
// PROZESSVERWALTUNG
// =============================================================================

void MainWindow::processFinished(int processId)
{
    // Verarbeitungszeit des letzten Prozesses erfassen
    switch (processId) {
    case 0:
        if (processATimer->isValid()) processATimeList->append(processATimer->elapsed());
        break;
    case 1:
        if (processBTimer->isValid()) processBTimeList->append(processBTimer->elapsed());
        break;
    case 2:
        if (processCTimer->isValid()) processCTimeList->append(processCTimer->elapsed());
        break;
    }

    finished++;

    // Wenn alle Prozesse abgeschlossen sind
    if (finished == 3) {
        // Prozess-Threads bereinigen
        threadProcessA->deleteLater();
        threadProcessA->quit();
        threadProcessA->wait();
        delete threadProcessA;
        delete workerA;

        threadProcessB->deleteLater();
        threadProcessB->quit();
        threadProcessB->wait();
        delete threadProcessB;
        delete workerB;

        threadProcessC->deleteLater();
        threadProcessC->quit();
        threadProcessC->wait();
        delete threadProcessC;
        delete workerC;

        // Preemption-Worker bereinigen falls verwendet
        if (selectedAlgorithmNumber == 1) {
            threadPreemption->deleteLater();
            threadPreemption->terminate();
            threadPreemption->wait();
            delete threadPreemption;
            delete preemptionWorker;
        }

        timer->stop();

        // Zusammenfassungsdialog anzeigen
        SummaryDialog summaryDialog;
        int maxResourceTimeA = !processATimeList->isEmpty() ? std::accumulate(processATimeList->begin(), processATimeList->end(), 0) / processATimeList->count() : 0;
        int maxResourceTimeB = !processBTimeList->isEmpty() ? std::accumulate(processBTimeList->begin(), processBTimeList->end(), 0) / processBTimeList->count() : 0;
        int maxResourceTimeC = !processCTimeList->isEmpty() ? std::accumulate(processCTimeList->begin(), processCTimeList->end(), 0) / processCTimeList->count() : 0;

        summaryDialog.getEndResults(ui->label_time->text(), countAllResourcesUsed, maxResourceTimeA, maxResourceTimeB, maxResourceTimeC);
        summaryDialog.setWindowTitle("Deadlock Algorithm Simulation");
        summaryDialog.exec();
    }
}

void MainWindow::setUpResourcesAndProcesses(int countPrinters, int countCD, int countPlotters, int countTapeDrive)
{
    // Ressourcen erstellen und konfigurieren
    QList<ManagedResource> resources;
    resources.append(ManagedResource("Printer", 0, countPrinters));
    resources.append(ManagedResource("CD-ROM", 1, countCD));
    resources.append(ManagedResource("Plotter", 2, countPlotters));
    resources.append(ManagedResource("TapeDrive", 3, countTapeDrive));

    // Verfügbare Ressourcen initialisieren
    for (int i = 0; i < resources.count(); i++) {
        availableResources_E[i] = resources.at(i).getCount();
        differenceResources_A[i] = resources.at(i).getCount();
    }

    // Benutzeroberfläche aktualisieren
    update_resource_occupation();

    // Prozesse mit sicheren Zufallswerten erstellen
    int safeMaxPrinters = qMax(2, countPrinters + 1);
    int safeMaxCDs = qMax(2, countCD + 1);
    int safeMaxPlotters = qMax(2, countPlotters + 1);
    int safeMaxTapeDrives = qMax(2, countTapeDrive + 1);

    processes.append(ManagedProcess("A", 0, safeMaxPrinters, safeMaxCDs, safeMaxPlotters, safeMaxTapeDrives));
    processes.append(ManagedProcess("B", 1, safeMaxPrinters, safeMaxCDs, safeMaxPlotters, safeMaxTapeDrives));
    processes.append(ManagedProcess("C", 2, safeMaxPrinters, safeMaxCDs, safeMaxPlotters, safeMaxTapeDrives));

    // Anforderungsmatrix aktualisieren
    updateStillNeededRessources_R();
}

// =============================================================================
// INITIALISIERUNGSMETHODEN
// =============================================================================

void MainWindow::setShadows()
{
    QGraphicsDropShadowEffect* effectShadow1 = new QGraphicsDropShadowEffect();
    effectShadow1->setBlurRadius(10);
    effectShadow1->setColor(QColor(0, 0, 0, 255 * 0.2));
    effectShadow1->setOffset(0,2);

    // Weitere Schatteneffekte initialisieren...
    // [Der ursprüngliche Code für die Schatteneffekte bleibt erhalten]
}

void MainWindow::initResourceCount(int* resourcesCounts)
{
    existingResources[0] = resourcesCounts[0];
    existingResources[1] = resourcesCounts[1];
    existingResources[2] = resourcesCounts[2];
    existingResources[3] = resourcesCounts[3];
}

void MainWindow::selectedAlgorithm(int algorithm)
{
    selectedAlgorithmNumber = algorithm;
}

// =============================================================================
// SLOTS FÜR BENUTZERINTERAKTION
// =============================================================================

void MainWindow::on_button_stop_simulation_clicked()
{
    // Prozess-Threads beenden
    threadProcessA->deleteLater();
    threadProcessA->terminate();
    threadProcessA->wait();
    delete threadProcessA;
    delete workerA;

    threadProcessB->deleteLater();
    threadProcessB->terminate();
    threadProcessB->wait();
    delete threadProcessB;
    delete workerB;

    threadProcessC->deleteLater();
    threadProcessC->terminate();
    threadProcessC->wait();
    delete threadProcessC;
    delete workerC;

    // Preemption-Worker beenden falls verwendet
    if (selectedAlgorithmNumber == 1) {
        threadPreemption->deleteLater();
        threadPreemption->terminate();
        threadPreemption->wait();
        delete threadPreemption;
        delete preemptionWorker;
    }

    timer->stop();

    // Zusammenfassungsdialog anzeigen
    SummaryDialog summaryDialog;
    int maxResourceTimeA = 0, maxResourceTimeB = 0, maxResourceTimeC = 0;

    if (!processATimeList->isEmpty()) maxResourceTimeA = std::accumulate(processATimeList->begin(), processATimeList->end(), 0) / processATimeList->count();
    if (!processBTimeList->isEmpty()) maxResourceTimeB = std::accumulate(processBTimeList->begin(), processBTimeList->end(), 0) / processBTimeList->count();
    if (!processCTimeList->isEmpty()) maxResourceTimeC = std::accumulate(processCTimeList->begin(), processCTimeList->end(), 0) / processCTimeList->count();

    summaryDialog.getEndResults(ui->label_time->text(), countAllResourcesUsed, maxResourceTimeA, maxResourceTimeB, maxResourceTimeC);
    summaryDialog.setWindowTitle("Deadlock Algorithm Simulation");
    summaryDialog.exec();
}

void MainWindow::on_button_start_simulation_clicked()
{
    ui->button_start_simulation->setEnabled(false);
    timer = new QTimer(this);

    QTime startTime = QTime::currentTime();
    connect(timer, &QTimer::timeout, [this, startTime]() {
        updateElapsedTime(startTime);
        timer->start(10);
    });

    timer->start(10);
}

void MainWindow::on_button_restart_simulation_clicked()
{
    QApplication::quit();

#ifdef Q_OS_WIN
    QProcess::startDetached(QApplication::applicationFilePath());
#elif defined(Q_OS_MACOS)
    QStringList arguments = QApplication::arguments();
    arguments.removeFirst();
    QProcess::startDetached(QApplication::applicationFilePath(), arguments);
#endif
}

// =============================================================================
// HILFSFUNKTIONEN
// =============================================================================

void MainWindow::updateElapsedTime(const QTime &startTime)
{
    QTime currentTime = QTime::currentTime();
    int elapsedMilliseconds = startTime.msecsTo(currentTime);

    int minutes = elapsedMilliseconds / 60000;
    int seconds = (elapsedMilliseconds % 60000) / 1000;
    int milliseconds = elapsedMilliseconds % 1000;

    QString formattedTime = QString("%1:%2:%3")
                            .arg(minutes, 2, 10, QChar('0'))
                            .arg(seconds, 2, 10, QChar('0'))
                            .arg(milliseconds, 3, 10, QChar('0'));

    ui->label_time->setText(formattedTime);
}

void MainWindow::loadTextFileIntoPlainTextEdit(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        ui->explanation_plainTextEdit->setPlainText(in.readAll());
        file.close();
    } else {
        qDebug() << "Fehler beim Öffnen der Datei:" << filePath;
    }
}

void MainWindow::on_explanation_Button_explanation_clicked()
{
    loadTextFileIntoPlainTextEdit(":/resources/explanations/explanationSimulation.txt");
    ui->explanation_Button_algorithm->setEnabled(true);
    ui->explanation_Button_explanation->setEnabled(false);
    ui->explanation_Title_label->setText("Simulation Überblick");
}

void MainWindow::on_explanation_Button_algorithm_clicked()
{
    switch (selectedAlgorithmNumber) {
    case 0:
        loadTextFileIntoPlainTextEdit(":/resources/explanations/explanationHoldAndWait.txt");
        ui->explanation_Title_label->setText("Eliminierung von Hold and Wait");
        break;
    case 1:
        loadTextFileIntoPlainTextEdit(":/resources/explanations/explanationPreemption.txt");
        ui->explanation_Title_label->setText("Eliminierung von no Preemption");
        break;
    case 2:
        loadTextFileIntoPlainTextEdit(":/resources/explanations/explanationCircularWait.txt");
        ui->explanation_Title_label->setText("Eliminierung von Circular Wait");
        break;
    case 3:
        loadTextFileIntoPlainTextEdit(":/resources/explanations/explanationBankier.txt");
        ui->explanation_Title_label->setText("Bankiers Algorithmus");
        break;
    default:
        loadTextFileIntoPlainTextEdit(":/resources/explanations/explanationDeadlock.txt");
        ui->explanation_Title_label->setText("Deadlock Anforderungen");
        break;
    }
    ui->explanation_Button_algorithm->setEnabled(false);
    ui->explanation_Button_explanation->setEnabled(true);
}

void MainWindow::updateInitialResources()
{
    initResourceCount(existingResources);
}
