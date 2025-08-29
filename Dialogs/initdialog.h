#ifndef INITDIALOG_H
#define INITDIALOG_H

#include <QDialog>

namespace Ui {
class InitDialog;
}

class InitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitDialog(QWidget *parent = nullptr);
    ~InitDialog();

    void getResourceCount();
    void getAlgorithm();

signals:
    void countsFinished(int* counts);
    void algorithmsFinished(int algorithm);

private slots:
    void on_openGithubButton_clicked();

    void on_startSimulationButton_clicked();

private:
    Ui::InitDialog *ui;
};

#endif // INITDIALOG_H


// Force MOC to run on this file
#if defined(Q_MOC_RUN)
#undef Q_MOC_RUN
#endif
