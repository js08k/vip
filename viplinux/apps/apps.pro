TARGET = apps

TEMPLATE = lib

CONFIG += QT

QMAKE_CC = /usr/bin/g++

INCLUDEPATH += \
        ../../vipdevel/vipsource/include \
        ../../vipdevel/apps/include \
        ../viplinux

SOURCES += \
        ../../vipdevel/apps/source/shell.c \
        ../../vipdevel/apps/source/telnet.c \
        ../../vipdevel/apps/source/httpd.c

HEADERS += \
        shell.h \
        telnet.h \
        httpd.h

DESTDIR = $${OUT_PWD}/../
