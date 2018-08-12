QT       += testlib

QT       -= gui

TARGET = tst_heaptest
CONFIG += console
CONFIG -= app_bundle

QMAKE_CC = /usr/bin/g++

TEMPLATE = app

INCLUDEPATH += \
    ../../../vipdevel/vipsource/include \
    ../../viplinux

DEPENDPATH += \
    $${INCLUDEPATH}

LIBS += \
        -L$${OUT_PWD}/../.. -lvipsource \
        -L$${OUT_PWD}/../.. -lapps

SOURCES += \
    $${PWD}/../../viplinux/vportable.cpp \
    tst_heaptest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
