OUTPUT=libmip-devel.a
#If header files are missing during the build, check that
#their path exists in the following.
PROJECT=/home/josh/workspace/vip/viparm/viparm

VIPHOME=/home/josh/workspace/vip/vipdevel/vipsource
VIPINCLUDE=$(VIPHOME)/include
APPINCLUDE=$(VIPHOME)/../apps/include
VIPSOURCE=$(VIPHOME)/source

RTOS_SOURCE_DIR=$(FreeRTOS)/Source
DRIVER_DIR=/home/josh/workspace/.StellarisDrivers
		
CC=/home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-gcc
ARCHIVE=/home/josh/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-ar
LDSCRIPT=standalone.ld

# should use --gc-sections but the debugger does not seem to be able to cope with the option.
#LINKER_FLAGS=-nostartfiles -Xlinker -o$(PROJECT).axf -Xlinker -M -Xlinker -Map=$(PROJECT).map -Xlinker --no-gc-sections

DEBUG=-g
OPTIM=-O0

#GCC_ARMCM3_LM3S102
CFLAGS=$(DEBUG) -I . -I $(RTOS_SOURCE_DIR)/include -I $(RTOS_SOURCE_DIR)/portable/GCC/ARM_CM3 \
		-D GCC_ARMCM3_LM3S8962 -D inline= -mthumb -mcpu=cortex-m3 $(OPTIM) -T $(LDSCRIPT) \
		-D PACK_STRUCT_END=__attribute\(\(packed\)\) -D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\)  \
		-I $(DRIVER_DIR) -I $(VIPINCLUDE) -I $(APPINCLUDE) -I $(PROJECT) -Wall

# vip makefile
SOURCE= \
	vtypes.c \
	varp.c \
	vbase.c \
	vfilter.c \
	vheap.c \
	vicmp.c \
	vipv4.c \
	vmang.c \
	vrandn.c \
	vstring.c \
	vsocket.c \
	vudp.c \
	vtcp.c
		
OBJS= $(SOURCE:%.c=%.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(ARCHIVE) rcs $(OUTPUT) $(OBJS)
	echo "$(OUTPUT) successfully created"

$(OBJS) : %.o : $(VIPSOURCE)/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -f $(OUTPUT)
	rm -f $(OBJS)
