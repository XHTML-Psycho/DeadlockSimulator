#ifndef SUMMARYDIALOG_H
#define SUMMARYDIALOG_H

#include <QDialog>

namespace Ui {

/**
 * @brief Class represents the Ending Dialog which appears with the result data after the simulation
 */
class SummaryDialog;
}

class SummaryDialog : public QDialog
{
    Q_OBJECT

public:

    explicit SummaryDialog(QWidget *parent = nullptr);

    ~SummaryDialog();

private slots:
    /**
     * @brief on_end_pushButton_clicked dialog closes the complete window on button click
     */
    void on_end_pushButton_clicked();

    void on_restart_pushButton_clicked();

public slots:

    void getEndResults(QString textFromRuntime, int numOfResources, int maxResourceTimeA, int maxResourceTimeB, int maxResourceTimeC);

private:
    Ui::SummaryDialog *ui;
};

#endif // ENDDIALOG_H
