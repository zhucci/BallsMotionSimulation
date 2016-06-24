#-------------------------------------------------
#
# Project created by QtCreator 2016-06-22T16:56:10
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BallsCollisionSim
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11
SOURCES += main.cpp\
        gui.cpp \
    ballcreator.cpp \
    ball.cpp \
    connectdialog.cpp \
    ballMotionSim.cpp \
    simcontroller.cpp \
    synchronizationmanager.cpp

HEADERS  += gui.h \
    ballcreator.h \
    ball.h \
    connectdialog.h \
    ballMotionSim.h \
    simcontroller.h \
    synchronizationmanager.h

FORMS    += gui.ui \
    addballdialog.ui \
    connectdialog.ui
