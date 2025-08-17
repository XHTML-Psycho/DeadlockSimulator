#include "processor.h"
#include "./ui_processor.h"

Processor::Processor(QWidget *parent): QMainWindow(parent), ui(new Ui::Processor) {
    ui->setupUi(this);
}

Processor::~Processor() {
    delete ui;
}
