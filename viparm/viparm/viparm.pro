TARGET = viparm
TEMPLATE = app

QT -= core
QT -= gui

DEFINES += __ARM__

LDSCRIPT= standalone.ld

OUTPUT=viparm

FreeRTOS=/home/josh/workspace/FreeRTOS
RTOS_SOURCE_DIR=$${FreeRTOS}/Source
DRIVER_DIR=/home/josh/workspace/.StellarisDrivers

QMAKE_CC = /home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc

# This is linux dependent. Todo: Find the windows equivalent
QMAKE_LINK = /home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc

QMAKE_AR = /home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-ar

QMAKE_LFLAGS = -nostartfiles -Xlinker -M --no-gc-sections

CCFLAG += \
    -Wall \
    -mthumb \
    -mcpu=cortex-m3 \
    -T $${LDSCRIPT} \
    -D inline= \
    -D GCC_ARMCM3_LM3S8962 \
    -D PACK_STRUCT_END=__attribute\(\(packed\)\) \
    -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
    -I $${RTOS_SOURCE_DIR}/include

LIBS += \
    $${DRIVER_DIR}/driverlib/gcc/libdriver.a \
    $${OUT_PWD}/../vipsource/libvipsource.a \
    $${OUT_PWD}/../apps/libvipsource.a


CFLAGS=$(DEBUG) -I .   \
                -I $${RTOS_SOURCE_DIR}/include \
                -I $(DRIVER_DIR) \
                -I $(VIPINCLUDE) \
                -I $(APPINCLUDE) \
                -I $(PROJECT) \
                -Wall

INCLUDEPATH = \
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
        startup.c \
        main.c \
        vportable.c

HEADERS += \
        vconfig.h
