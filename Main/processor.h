#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {class Processor;}
QT_END_NAMESPACE

class Processor : public QMainWindow {
    Q_OBJECT

public:
    Processor(QWidget *parent = nullptr);
    ~Processor();

private:
    Ui::Processor *ui;
};
#endif // PROCESSOR_H
