TARGET = apps

TEMPLATE = lib
CONFIG += staticLib

QT -= core
QT -= gui

LDSCRIPT= ../standalone.ld

OUTPUT=apps.a

FreeRTOS=/home/josh/workspace/FreeRTOS
RTOS_SOURCE_DIR=$${FreeRTOS}

TARGET = vipsource

TEMPLATE = lib

CONFIG += QT

DEFINES += __ARM__

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
        ../../vipdevel/apps/source/httpd.c \
        ../../vipdevel/apps/source/shell.c \
        ../../vipdevel/apps/source/telnet.c

HEADERS += \
        ../../vipdevel/apps/include/httpd.h \
        ../../vipdevel/apps/include/shell.h \
        ../../vipdevel/apps/include/telnet.h

#DESTDIR = $${OUT_PWD}/../

