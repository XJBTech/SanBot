#
#   ____    ____    __   __   ______  ______
#  /\  _`\ /\  _`\ /\ \ /\ \ /\__  _\/\  _  \
#  \ \ \/\ \ \ \L\_\ `\`\/'/'\/_/\ \/\ \ \L\ \
#   \ \ \ \ \ \  _\L`\/ > <     \ \ \ \ \  __ \
#    \ \ \_\ \ \ \L\ \ \/'/\`\   \ \ \ \ \ \/\ \
#     \ \____/\ \____/ /\_\\ \_\  \ \_\ \ \_\ \_\
#      \/___/  \/___/  \/_/ \/_/   \/_/  \/_/\/_/
#
#  Originally created by Dexta Robotics.
#  Copyright <C> Dexta Robotics, 2015.
#  All rights reserved.
#

#ST-Link configuration
STFLASH 			?= st-flash
CROSS_COMPILE 		?= arm-none-eabi-
PYTHON2 			?= python

CLOAD 				?= 0
DEBUG 				?= 0

PLATFORM			?= SANBOT
DEVICE 				?= A
REV 				?= A
#DEVICE 			?= DONGLE

PLATFORM_DEVICE		?= $(PLATFORM)_$(DEVICE)
PLATFORM_REV 		?= $(PLATFORM)_$(DEVICE)_$(REV)

ifeq ($(PLATFORM_DEVICE), SANBOT_A)
STFLASH_ADDRESS		?= 0x8000000
endif

ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
STFLASH_ADDRESS		?= 0x8000000
endif

ifeq ($(PLATFORM_DEVICE), SANBOT_A)
LINKER_DIR = scripts/F103/linker
ST_OBJ_DIR  = scripts/F103
endif

ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
LINKER_DIR = scripts/F103/linker
ST_OBJ_DIR  = scripts/F103
endif

STLIB = lib
SBN1LIB = ../sbn1

# St Lib
VPATH_SANBOT_A += $(STLIB)/CMSIS/Core/CM3
VPATH_SANBOT_A += $(STLIB)/CMSIS/Core/CM3/startup/gcc
# VPATH_SANBOT_A += $(STLIB)/STM32_CPAL_Driver/src
# VPATH_SANBOT_A += $(STLIB)/STM32_CPAL_Driver/devices/stm32f10x
CRT0_SANBOT_A = startup_stm32f10x_hd.o system_stm32f10x.o

VPATH_SANBOT_DONGLE += $(STLIB)/CMSIS/Core/CM3
VPATH_SANBOT_DONGLE += $(STLIB)/CMSIS/Core/CM3/startup/gcc
CRT0_SANBOT_DONGLE = startup_stm32f10x_hd.o system_stm32f10x.o

# Should maybe be in separate file?
-include $(ST_OBJ_DIR)/st_obj.mk

VPATH += init hal/src modules/src utils/src drivers/src $(SBN1LIB)
VPATH_SANBOT_A += platform/sanbot_a drivers/eMPL
VPATH_SANBOT_DONGLE += platform/sanbot_dongle drivers/eMPL

ifeq ($(PLATFORM_DEVICE), SANBOT_A)
VPATH += $(VPATH_SANBOT_A)
endif

ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
VPATH += $(VPATH_SANBOT_DONGLE)
endif

#core
# PROJ_OBJ += core_cm3.o

# Init
PROJ_OBJ += main.o
PROJ_OBJ_SANBOT_A += platform_sanbot_a.o
PROJ_OBJ_SANBOT_DONGLE += platform_sanbot_dongle.o

# Drivers
PROJ_OBJ_SANBOT_A += led.o nrf24l01.o
# PROJ_OBJ_SANBOT_A += led.o nrf24l01.o i2cs.o mpu9150.o inv_mpu.o adc.o tim.o inv_mpu_dmp_motion_driver.o
# PROJ_OBJ_SANBOT1 += adc.o i2cs.o MPU9150.o nRF24L01.o tim.o inv_mpu_dmp_motion_driver.o inv_mpu.o
PROJ_OBJ_SANBOT_DONGLE += led.o nrf24l01.o

# Hal
PROJ_OBJ_SANBOT_A += exti.o nvic.o uart_fifo.o rcc.o sbn1.sanbot_a.o
# PROJ_OBJ_SANBOT1 +=  SBN1.o
PROJ_OBJ_SANBOT_DONGLE += exti.o nvic.o uart_fifo.o rcc.o sbn1.sanbot_dongle.o

# Modules
PROJ_OBJ += system.o

# Utilities
PROJ_OBJ += eprintf.o debug.o stub.o

OBJ = $(FREERTOS_OBJ) $(PORT_OBJ) $(ST_OBJ) $(PROJ_OBJ)

ifeq ($(PLATFORM_DEVICE), SANBOT_A)
OBJ += $(CRT0_SANBOT_A) $(ST_OBJ_SANBOT_A) $(PROJ_OBJ_SANBOT_A)
endif

ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
OBJ += $(CRT0_SANBOT_DONGLE) $(ST_OBJ_SANBOT_DONGLE) $(PROJ_OBJ_SANBOT_DONGLE)
endif

ifdef P
  C_PROFILE = -D P_$(P)
endif

############### Compilation configuration ################
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy

INCLUDES  = -I$(FREERTOS)/include -I$(PORT) -I. -I$(SBN1LIB)
INCLUDES += -Iconfig -Ihal/interface -Imodules/interface -Idrivers/eMPL
INCLUDES += -Iutils/interface -Idrivers/interface -Iplatform
INCLUDES += -I$(STLIB)/CMSIS/Include

INCLUDES_SANBOT_A += -I$(STLIB)/STM32F10x_StdPeriph_Driver/inc
INCLUDES_SANBOT_A += -I$(STLIB)/CMSIS/Core/CM3
# INCLUDES_SANBOT_A += -I$(STLIB)/STM32_CPAL_Driver/inc
# INCLUDES_SANBOT_A += -I$(STLIB)/STM32_CPAL_Driver/devices/stm32f10x
INCLUDES_SANBOT_DONGLE += -I$(STLIB)/STM32F10x_StdPeriph_Driver/inc
INCLUDES_SANBOT_DONGLE += -I$(STLIB)/CMSIS/Core/CM3

ifeq ($(PLATFORM_DEVICE), SANBOT_A)
	PROCESSOR = -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
endif

ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
	PROCESSOR = -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
endif

#Flags required by the ST library
STFLAGS += -DPLATFORM_$(PLATFORM) -DPLATFORM_DEVICE_$(PLATFORM_DEVICE) -DPLATFORM_REV_$(PLATFORM_REV)

STFLAGS_SANBOT_A = -DSTM32F10X_HD -DHSE_VALUE=8000000 -include stm32f10x_conf.h
STFLAGS_SANBOT_DONGLE = -DSTM32F10X_MD -DHSE_VALUE=8000000 -include stm32f10x_conf.h

ifeq ($(DEBUG), 1)
  CFLAGS += -O0 -g3 -DDEBUG
else
  CFLAGS += -Os -g3
endif

ifeq ($(LTO), 1)
  CFLAGS += -flto
endif

ifeq ($(USE_ESKYLINK), 1)
  CFLAGS += -DUSE_ESKYLINK
endif

# CFLAGS += -DBOARD_REV_$(REV)

CFLAGS += $(PROCESSOR) $(INCLUDES) $(STFLAGS)
ifeq ($(PLATFORM_DEVICE), SANBOT_A)
CFLAGS += $(INCLUDES_SANBOT_A) $(STFLAGS_SANBOT_A)
endif
ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
CFLAGS += $(INCLUDES_SANBOT_DONGLE) $(STFLAGS_SANBOT_DONGLE)
endif


CFLAGS += -Wall -fno-strict-aliasing $(C_PROFILE) -Wunused-but-set-variable
# Compiler flags to generate dependency files:
CFLAGS += -MD -MP -MF $(BIN)/dep/$(@).d -MQ $(@)
#Permits to remove un-used functions and global variables from output file
CFLAGS += -ffunction-sections -fdata-sections

CFLAGS += -Wno-format -Wno-unused-function

ASFLAGS = $(PROCESSOR) $(INCLUDES)
LDFLAGS = --specs=nano.specs $(PROCESSOR) -Wl,-Map=$(PROG).map,--cref,--gc-sections

#Flags required by the ST library
LDFLAGS += -T $(LINKER_DIR)/FLASH.ld
LOAD_ADDRESS = 0x8000000

ifeq ($(LTO), 1)
  LDFLAGS += -Os -flto -fuse-linker-plugin
endif

#Program name
ifeq ($(PLATFORM_DEVICE), SANBOT_A)
PROG = SANBOT_A
endif
ifeq ($(PLATFORM_DEVICE), SANBOT_DONGLE)
PROG = SANBOT_DONGLE
endif
#Where to compile the .o
BIN = bin
VPATH += $(BIN)

#Dependency files to include
DEPS := $(foreach o,$(OBJ),$(BIN)/dep/$(o).d)

#################### Targets ###############################


all: view_target build cloc
build: compile size
compile: $(PROG).hex $(PROG).bin $(PROG).dfu
	@echo "\033[44;37m$$ build $(PLATFORM_REV) successfully. \033[0m"

size: compile
	@$(SIZE) -B $(PROG).elf

#Flash the stm.
flash: all
	$(STFLASH) write $(PROG).bin $(STFLASH_ADDRESS)
	@echo "\033[44;37m$$ done. \033[0m"

#$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
#                -c "flash write_image erase $(PROG).elf" -c "verify_image $(PROG).elf" -c "reset run" -c shutdown

view_target:
	@echo "  PLATFORM = $(PLATFORM)"
	@echo "  PLATFORM_DEVICE = $(PLATFORM_DEVICE)"
	@echo "  PLATFORM_REV = $(PLATFORM_REV)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "\033[41;37m$$ start to build $(PLATFORM_REV) \033[0m"


erase:
	$(STFLASH) erase
	@echo "\033[44;37m$$ done. \033[0m"

#Print preprocessor #defines
prep:
	@$(CC) -dD

clean_bin: clean
	@rm -f *.bin *.dfu *.elf *.hex *.map
	@echo "\033[44;37m$$ clean successfully. \033[0m"

new: clean_bin all

cloc:
	@echo "\033[;32m"
	@ cloc . --exclude-dir=eMPL,lib --ignore-whitespace --exclude-lang="D"
	@echo "\033[0m"

include scripts/targets.mk

#include dependencies
-include $(DEPS)
