QT       += core gui network
QT       += charts
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialog.cpp \
    main.cpp \
    mainwindow.cpp \
    modelARX.cpp \
    qcustomplot.cpp \
    regulatorPID.cpp \
    sprzezenie.cpp \
    sygnal.cpp \
    symulator.cpp

HEADERS += \
    dialog.h \
    mainwindow.h \
    modelARX.h \
    qcustomplot.h \
    regulatorPID.h \
    sprzezenie.h \
    sygnal.h \
    symulator.h

FORMS += \
    dialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
