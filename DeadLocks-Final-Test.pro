QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Algorithms/bankersalgorithm.cpp \
    Algorithms/circularwaithandler.cpp \
    Algorithms/directallocationsim.cpp \
    Algorithms/holdandwaithandler.cpp \
    Algorithms/resource_manager_api.cpp \
    Algorithms/resourcereallocator.cpp \
    Algorithms/strictlockpolicy.cpp \
    Dialogs/initdialog.cpp \
    Dialogs/summarydialog.cpp \
    Main/main.cpp \
    Main/mainwindow.cpp \
    Objects/managedprocess.cpp \
    Objects/managedresource.cpp \
    Objects/processhandler.cpp

HEADERS += \
    Algorithms/bankersalgorithm.h \
    Algorithms/circularwaithandler.h \
    Algorithms/directallocationsim.h \
    Algorithms/holdandwaithandler.h \
    Algorithms/resource_manager_api.h \
    Algorithms/resourcereallocator.h \
    Algorithms/strictlockpolicy.h \
    Dialogs/initdialog.h \
    Dialogs/summarydialog.h \
    Main/mainwindow.h \
    Objects/managedprocess.h \
    Objects/managedresource.h \
    Objects/processhandler.h

FORMS += \
    Dialogs/initdialog.ui \
    Dialogs/summarydialog.ui \
    Main/mainwindow.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    resources/checkBox_off.png \
    resources/checkBox_off_hover.png \
    resources/checkBox_on.png \
    resources/checkBox_on_hover.png \
    resources/reset_Icon.png \
    resources/restet_Icon.png
