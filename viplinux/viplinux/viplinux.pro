TARGET = viplinux

TEMPLATE = app

CONFIG += QT

QMAKE_CC = /usr/bin/g++

INCLUDEPATH += \
    ../../include/vipsource \
    ../../include/apps

DEPENDPATH += \
    $${INCLUDEPATH}

LIBS += \
        -lpcap \
        -L$${OUT_PWD}/../../lib -lvipsource -lapps

SOURCES += \
    vportable.cpp \
    main.cpp

HEADERS += \
    vconfig.h \
    main.h

QMAKE_POST_LINK += /usr/bin/sudo /sbin/setcap cap_net_raw,cap_net_admin=eip $${TARGET}
