ifndef ROOT_DIR
$(error Setup script settings.sh has not been sourced, aborting)
endif

include $(SW_SOC_ROOT)/common/config.mk

###############################################################################################
# Project structure
###############################################################################################
APP_DIR      := $(CURDIR)
APP_NAME     := $(notdir $(CURDIR))

KERNEL_DIR   := $(SW_SOC_ROOT)/projects/freertos/FreeRTOS-Kernel
HEAP_PROFILE ?= 1

BUILD_DIR := $(CURDIR)/build

###############################################################################################
# Kernel sources (shared)
###############################################################################################
KERNEL_SRCS  := $(wildcard $(KERNEL_DIR)/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/MemMang/heap_$(HEAP_PROFILE).c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.S)

KERNEL_OBJS := $(KERNEL_SRCS:.c=.o)
KERNEL_OBJS := $(KERNEL_OBJS:.S=.o)

###############################################################################################
# CFLAGS
###############################################################################################
CFLAGS   += -Wall -Wextra -ffreestanding

INCLUDES := -I$(APP_DIR) \
            -I$(KERNEL_DIR)/include \
            -I$(KERNEL_DIR)/portable/GCC/RISC-V \
            -I$(KERNEL_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions \
            -I$(SW_SOC_ROOT)/lib/uninasoc/inc \
            -I$(SW_SOC_ROOT)/lib/tinyio/inc

ifeq ($(DEBUG), 1)
CFLAGS  += -g
endif

CFLAGS  += $(INCLUDES)

###############################################################################################
# Linker flags
###############################################################################################
LDFLAGS  = -nostdlib -nostartfiles -Tlinker.ld
LDFLAGS += $(SW_SOC_ROOT)/lib/uninasoc/lib/libuninasoc.a $(SW_SOC_ROOT)/lib/tinyio/lib/tinyio.a

###############################################################################################
# Application sources
###############################################################################################
APP_SRCS := $(wildcard $(APP_DIR)/*.c) $(wildcard $(APP_DIR)/*.S)
APP_OBJS := $(APP_SRCS:.c=.o)
APP_OBJS := $(APP_OBJS:.S=.o)

###############################################################################################
# Final artifacts
###############################################################################################
ELF := $(BUILD_DIR)/$(APP_NAME).elf
BIN := $(BUILD_DIR)/$(APP_NAME).bin

# Create build directory for final outputs
$(shell mkdir -p $(BUILD_DIR))

###############################################################################################
# Rules
###############################################################################################
all: $(BIN)

# Compile kernel sources in place
%.o: %.c
	@echo "  KERNEL CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "  KERNEL AS  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile app sources in place
$(APP_DIR)/%.o: $(APP_DIR)/%.c
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(APP_DIR)/%.o: $(APP_DIR)/%.S
	@echo "  AS    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link
$(ELF): $(APP_OBJS) $(KERNEL_OBJS)
	@mkdir -p $(dir $@)
	@echo "  LD    $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

# Binary
$(BIN): $(ELF)
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

clean:
	@rm -f $(APP_OBJS) $(KERNEL_OBJS)
	@rm -rf $(BUILD_DIR)

.PHONY: all clean
