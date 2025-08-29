#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <Objects/ManagedResource.h>
#include <Objects/ManagedProcess.h>
#include <Objects/ProcessHandler.h>
#include <Algorithms/resourcereallocator.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The MainWindow class represents the main window of the simulation application.
 * It initializes resources, manages processes, and handles UI interactions.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a MainWindow object
     * @param parent The parent widget (default is nullptr)
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor for the MainWindow
     */
    ~MainWindow();

    /**
     * @brief Sets up the initial resources and processes
     * @param countPrinters The number of printers
     * @param countCD The number of CD-ROM drives
     * @param countPlotters The number of plotters
     * @param countTapeDrive The number of tape drives
     */
    void setUpResourcesAndProcesses(int countPrinters, int countCD, int countPlotters, int countTapeDrive);

    /**
     * @brief Loads a text file into a plain text edit widget in the UI
     * @param filePath Path to the text file
     */
    void loadTextFileIntoPlainTextEdit(const QString &filePath);

private slots:
    ///< UI update slots
    void update_occupation_matrix();
    void update_needed_matrix();
    void update_resource_occupation();
    void update_resource_occupation_list();
    void updateElapsedTime(const QTime &startTime);
    void updateInitialResources();

    ///< resource configuration
    void setShadows();
    void initResourceCount(int* resourcesCounts);
    void selectedAlgorithm(int algorithm);

    ///< update variables when occupation changes
    void updateStillNeededRessources_R();

    ///< slots for resource management (worker threads)
    void reserveResources(int process, int resource, int count);
    void releaseResources(int process, int resource, int count, bool notProcessedYet);
    void processFinished(int processId);

    ///< button click slots
    void on_button_stop_simulation_clicked();
    void on_button_start_simulation_clicked();
    void on_button_restart_simulation_clicked();

    ///< explanation buttons
    void on_explanation_Button_explanation_clicked();
    void on_explanation_Button_algorithm_clicked();

private:
    Ui::MainWindow *ui;

    // Threads simulating processes A–C and a preemption worker
    QThread *threadProcessA;
    QThread *threadProcessB;
    QThread *threadProcessC;
    QThread *threadPreemption;

    // Workers to represent processes and preemption handling
    ProcessHandler *workerA;
    ProcessHandler *workerB;
    ProcessHandler *workerC;
    ResourceReallocator *preemptionWorker;
};

#endif // MAINWINDOW_H
