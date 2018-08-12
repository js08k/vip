TARGET = vipsource

TEMPLATE = lib
CONFIG += staticLib

QT -= core
QT -= gui

DEFINES += __ARM__

LDSCRIPT= ../standalone.ld

OUTPUT=vipsource.a

FreeRTOS=/home/josh/workspace/FreeRTOS
RTOS_SOURCE_DIR=$${FreeRTOS}

TARGET = vipsource

TEMPLATE = lib

CONFIG += QT

QMAKE_CC = /home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc

# This is linux dependent. Todo: Find the windows equivalent
QMAKE_AR = /home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-ar rcs

CCFLAG += \
    -Wall \
    -mthumb \
    -mcpu=cortex-m3 \
    -T $${LDSCRIPT} \
    -D inline= \
    -D GCC_ARMCM3_LM3S8962 \
    -D PACK_STRUCT_END=__attribute\(\(packed\)\) \
    -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)



CFLAGS=$(DEBUG) -I .   \
                -I $(DRIVER_DIR) -I $(VIPINCLUDE) -I $(APPINCLUDE) -I $(PROJECT) -Wall

INCLUDEPATH += \
    $${RTOS_SOURCE_DIR}/include \
    $${RTOS_SOURCE_DIR}/portable/GCC/ARM_CM3 \
    $${DRIVER_DIR} \
    ../../vipdevel/vipsource/include \
    ../../vipdevel/apps/include \
    ../viparm


OTHER_FILES += \
    $${LDSCRIPT} \
    Makefile

SOURCES += \
        ../../vipdevel/vipsource/source/vbase.c \
        ../../vipdevel/vipsource/source/vheap.c \
        ../../vipdevel/vipsource/source/vipv4.c \
        ../../vipdevel/vipsource/source/vsocket.c \
        ../../vipdevel/vipsource/source/vtypes.c \
        ../../vipdevel/vipsource/source/varp.c \
        ../../vipdevel/vipsource/source/vfilter.c \
        ../../vipdevel/vipsource/source/vicmp.c \
        ../../vipdevel/vipsource/source/vmang.c \
        ../../vipdevel/vipsource/source/vrandn.c \
        ../../vipdevel/vipsource/source/vstring.c

HEADERS += \
        ../../vipdevel/vipsource/include/vfilter.h \
        ../../vipdevel/vipsource/include/vipv4.h \
        ../../vipdevel/vipsource/include/vstring.h \
        ../../vipdevel/vipsource/include/varp.h \
        ../../vipdevel/vipsource/include/vheap.h \
        ../../vipdevel/vipsource/include/vmang.h \
        ../../vipdevel/vipsource/include/vrandn.h \
        ../../vipdevel/vipsource/include/vtcp.h \
        ../../vipdevel/vipsource/include/vbase.h \
        ../../vipdevel/vipsource/include/vicmp.h \
        ../../vipdevel/vipsource/include/voverlay.h \
        ../../vipdevel/vipsource/include/vsocket.h \
        ../../vipdevel/vipsource/include/vtypes.h \
        ../../vipdevel/vipsource/include/vportable.h
