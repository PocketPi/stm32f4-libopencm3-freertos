STM32CUBEPROG ?= STM32_Programmer_CLI

DEVICE=stm32f411ceu6

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

OBJ_DIR = obj
BIN_DIR = bin
SRC_DIR = src
SUBMODULES_DIR = submodules

OPENCM3_DIR := $(SUBMODULES_DIR)/libopencm3
FREERTOS_DIR := $(SUBMODULES_DIR)/FreeRTOS

SMBUS_DIR := $(SUBMODULES_DIR)/SMBus
INA323_DIR := $(SRC_DIR)/ina232


# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)
PROJECT_NAME=main

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
SRC_FILES += $(wildcard $(SMBUS_DIR)/*.c)
SRC_FILES += $(wildcard $(INA323_DIR)/*.c)

FREERTOS_SRC_FILES := $(FREERTOS_DIR)/tasks.c
FREERTOS_SRC_FILES += $(FREERTOS_DIR)/list.c
FREERTOS_SRC_FILES += $(FREERTOS_DIR)/queue.c
FREERTOS_SRC_FILES += $(FREERTOS_DIR)/timers.c
FREERTOS_SRC_FILES += $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/port.c
FREERTOS_SRC_FILES += $(FREERTOS_DIR)/portable/MemMang/heap_4.c

INCLUDES := -I$(realpath config)
INCLUDES += -I$(realpath src)
INCLUDES += -I$(OPENCM3_DIR)/include
INCLUDES += -I$(FREERTOS_DIR)/include
INCLUDES += -I$(FREERTOS_DIR)/portable/GCC/ARM_CM4F
INCLUDES += -I$(SMBUS_DIR)
INCLUDES += -I$(INA323_DIR)

OBJECTS := $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.c=.o))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(FREERTOS_SRC_FILES:.c=.o))

# Used libraries
DEFS		+= $(INCLUDES)
LDFLAGS		+= -L$(OPENCM3_DIR)/lib
LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDLIBS 		+= -specs=nosys.specs


include $(OPENCM3_DIR)/mk/genlink-config.mk

ifeq (,$(DEVICE))
LDLIBS += -l$(OPENCM3_LIB)
endif

# error if not using linker script generator
ifeq (,$(DEVICE))
$(LDSCRIPT):
ifeq (,$(wildcard $(LDSCRIPT)))
    $(error Unable to find specified linker script: $(LDSCRIPT))
endif
else
# if linker script generator was used, make sure it's cleaned.
GENERATED_BINS += $(LDSCRIPT)
endif

# Target-specific flags
FP_FLAGS	?= -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS	= -mthumb -mcpu=cortex-m4 $(FP_FLAGS)

# Compiler configuration
PREFIX		?= arm-none-eabi

CC		:= $(PREFIX)-gcc
CXX		:= $(PREFIX)-g++
LD		:= $(PREFIX)-gcc
AR		:= $(PREFIX)-ar
AS		:= $(PREFIX)-as
SIZE	:= $(PREFIX)-size
OBJCOPY	:= $(PREFIX)-objcopy
OBJDUMP	:= $(PREFIX)-objdump
GDB		:= $(PREFIX)-gdb
STFLASH	 = $(shell which st-flash)
OPT		:= -Os
DEBUG	:= -ggdb3
CSTD	?= -std=c99

# C flags
TGT_CFLAGS	+= $(OPT) $(CSTD) $(DEBUG)
TGT_CFLAGS	+= $(ARCH_FLAGS)
TGT_CFLAGS	+= -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS	+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS	+= -fno-common -ffunction-sections -fdata-sections
TGT_CFLAGS  += -Werror -Wpedantic

# C & C++ preprocessor common flags
TGT_CPPFLAGS	+= -MD
TGT_CPPFLAGS	+= -Wall -Wundef
TGT_CPPFLAGS	+= $(DEFS)

# Linker flags
TGT_LDFLAGS		+= --static -nostartfiles
TGT_LDFLAGS		+= -T$(LDSCRIPT)
TGT_LDFLAGS		+= $(ARCH_FLAGS) $(DEBUG)
TGT_LDFLAGS		+= -Wl,-Map=$(BIN_DIR)/$(*).map -Wl,--cref
TGT_LDFLAGS		+= -Wl,--gc-sections
ifeq ($(V),1)
TGT_LDFLAGS		+= -Wl,--print-gc-sections
endif
TGT_LDFLAGS += -Wl,--print-memory-usage
TGT_LDFLAGS += -u _printf_float

.PHONY: all clean flash
.SECONDARY: $(OBJECTS) $(BIN_DIR)/$(PROJECT_NAME).elf

all: $(BIN_DIR)/$(PROJECT_NAME).bin

$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	@printf "  OBJCOPY\t$@\n"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(BIN_DIR)/%.elf: $(OBJECTS) $(LDSCRIPT)
	@printf "  LD\t$@\n"
	@mkdir -p $(dir $@)
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@printf "  CLEAN\tbin\n"
	$(Q)rm -Rf bin/*
	@printf "  CLEAN\tobj\n"
	$(Q)rm -Rf obj/*

flash: $(BIN_DIR)/$(PROJECT_NAME).elf
	@printf "  FLASH\t$<\n"
	$(Q)$(STM32CUBEPROG) -c port=usb1 reset=HWrst -w $< -v

format:
	$(Q)clang-format -i -style=file --verbose $(SRC_FILES)

include $(OPENCM3_DIR)/mk/genlink-rules.mk
