#include "processor.h"
#include "./ui_processor.h"
#include "QFile"

Processor::Processor(QWidget *parent): QMainWindow(parent), ui(new Ui::Processor) {
    ui->setupUi(this);
/*
    QFile file(":/origin/1.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);
*/
}

Processor::~Processor() {
    delete ui;
}
