TARGET = apps

TEMPLATE = lib

CONFIG += QT

QMAKE_CC = /usr/bin/g++

INCLUDEPATH += \
        ../../vipdevel/vipsource/include \
        ../../vipdevel/apps/include \
        ../viplinux

SOURCES += \
        shell.c \
        telnet.c \
        httpd.c

HEADERS += \
        vconfig.h \
        ../../include/apps/shell.h \
        ../../include/apps/telnet.h \
        ../../include/apps/httpd.h

DESTDIR = $${OUT_PWD}/..
