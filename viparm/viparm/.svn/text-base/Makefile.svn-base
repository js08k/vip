OUTPUT=viparm

PROJECT=/home/josh/workspace/vip/viparm

MIPHOME=/home/josh/workspace/vip/vipdevel/vipsource
MIPINC=$(MIPHOME)/include
MIPLIB=$(PROJECT)/vipsource/libmip-devel.a
#end mIP directories and source

APPHOME=/home/josh/workspace/vip/vipdevel/apps
APPINC=$(APPHOME)/include
APPLIB=$(PROJECT)/apps/apps.a

RTOS_SOURCE_DIR=$(FreeRTOS)/Source
DRIVER_DIR=/home/josh/workspace/.StellarisDrivers

CC=/home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc
OBJCOPY=/home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-objcopy
LDSCRIPT=standalone.ld

# should use --gc-sections but the debugger does not seem to be able to cope with the option.
LINKER_FLAGS=-nostartfiles -Xlinker -o$(OUTPUT).axf -Xlinker -M -Xlinker -Map=$(OUTPUT).map -Xlinker --no-gc-sections -L$(APPLIB) -L$(MIPLIB)

DEBUG=-g
OPTIM=-O0

#GCC_ARMCM3_LM3S102
CFLAGS= $(DEBUG) -I . -I $(RTOS_SOURCE_DIR)/include \
	-I $(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3 \
	-D GCC_ARMCM3_LM3S8962 -D inline= -mthumb -mcpu=cortex-m3 \
	$(OPTIM) -T $(LDSCRIPT) -D PACK_STRUCT_END=__attribute\(\(packed\)\) \
	-D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)  \
	-I $(DRIVER_DIR) -I $(MIPINC) -L $(MIPLIB) -I $(PROJECT) \
	-L$(APPLIB) -I$(APPINC) -Wall

SOURCE=	main.c \
	vportable.c 

LIBS= $(DRIVER_DIR)/driverlib/gcc/libdriver.a

OBJS = $(SOURCE:.c=.o)

all: $(OUTPUT).bin

$(OUTPUT).bin : $(OUTPUT).axf
	$(OBJCOPY) $(OUTPUT).axf -O binary $(OUTPUT).bin

$(OUTPUT).axf : $(OBJS)  startup.o Makefile $(MIPLIB) $(APPLIB)
	$(CC) $(CFLAGS) $(OBJS) startup.o $(MIPLIB) $(APPLIB) $(LIBS) $(LINKER_FLAGS)

$(OBJS) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@
	
startup.o : startup.c Makefile
	$(CC) -c $(CFLAGS) -O1 startup.c -o startup.o

$(MIPLIB):
	make -C ../vipsource/ all
	
$(APPLIB):
	make -C ../apps/ all
	
clean :
	make -C ../vipsource/ clean
	make -C ../apps/ clean
	rm -f $(OBJS)
	rm -f $(OUTPUT).bin
	rm -f $(OUTPUT).axf
	rm -f $(OUTPUT).map

