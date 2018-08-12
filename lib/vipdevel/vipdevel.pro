TARGET = vipsource

TEMPLATE = lib

CONFIG += QT

QMAKE_CC = /usr/bin/g++

INCLUDEPATH += \
        ../../vipdevel/vipsource/include \
        ../../vipdevel/apps/include \
        ../viplinux

SOURCES += \
        vbase.c \
        vheap.c \
        vipv4.c \
        vsocket.c \
        vtypes.c \
        varp.c \
        vfilter.c \
        vicmp.c \
        vmang.c \
        vrandn.c \
        vstring.c \
        vudp.c \
        vtcp.c

HEADERS += \
        vconfig.h \
        ../../include/vipsource/vfilter.h \
        ../../include/vipsource/vipv4.h \
        ../../include/vipsource/vstring.h \
        ../../include/vipsource/varp.h \
        ../../include/vipsource/vheap.h \
        ../../include/vipsource/vmang.h \
        ../../include/vipsource/vrandn.h \
        ../../include/vipsource/vtcp.h \
        ../../include/vipsource/vbase.h \
        ../../include/vipsource/vicmp.h \
        ../../include/vipsource/voverlay.h \
        ../../include/vipsource/vsocket.h \
        ../../include/vipsource/vtypes.h \
        ../../include/vipsource/vportable.h \
        ../../include/vipsource/vudp.h

DESTDIR = $${OUT_PWD}/..
