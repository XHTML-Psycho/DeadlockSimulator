#include "processor.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Processor w;
    w.show();
    return a.exec();
}
