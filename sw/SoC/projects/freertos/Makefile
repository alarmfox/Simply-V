# This Makefile provides rules to compile and link application in app/ with the FreeRTOS kernel and 
# the UninaSoC HAL. Applications are supposed to be in app/<app-name> and will be built in build/<app-name>.
#
# Users may want to customize the Heap Profile by passing HEAP_PROFILE to make variable (default is 1).
#
# Platform specific extensions need to be enabled in the $(KERNEL_DIR)/portable/GCC/RISC-V/chip_specific_extensions/
# (default RISCV_no_extensions). Users may want to define extra variable in RISCV_no_extensions files
# or to switch to another platform.
#
# Author: Giuseppe Capasso <giuseppe.capasso17@studenti.unina.it>

include ../../common/config.mk

###############################################################################################
# Project structure
###############################################################################################
APP_DIR      := app
KERNEL_DIR   := FreeRTOS-Kernel
BUILD_DIR    := build
HEAP_PROFILE ?= 1

###############################################################################################
# Kernel sources (shared)
###############################################################################################
KERNEL_SRCS  := $(wildcard $(KERNEL_DIR)/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/MemMang/heap_$(HEAP_PROFILE).c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.S)

KERNEL_OBJS := $(BUILD_DIR)/startup.o \
               $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.S,$(BUILD_DIR)/%.o,$(KERNEL_SRCS)))

###############################################################################################
# CFLAGS
###############################################################################################
CFLAGS   += -Wall -Wextra -ffreestanding

INCLUDES := -I. \
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
# Discover applications
###############################################################################################
APP_DIRS  := $(wildcard $(APP_DIR)/*)     # app/basic app/other
APP_NAMES := $(notdir $(APP_DIRS))        # basic other

###############################################################################################
# Build targets (one explicit pair of targets per app)
###############################################################################################
# default: all app bins
BIN_TARGETS := $(foreach a,$(APP_NAMES),$(BUILD_DIR)/$(a)/$(a).bin)
all: $(BIN_TARGETS)

# Helper: template that creates explicit targets per app:
#  build/<app>/<app>.elf build/<app>/<app>.bin build/<app>/%.o
define MAKE_APP_RULES
# sources for app '$(1)'
APP_SRCS_$(1) := $(wildcard $(APP_DIR)/$(1)/*.c) $(wildcard $(APP_DIR)/$(1)/*.S)
APP_OBJS_$(1) := $(patsubst $(APP_DIR)/$(1)/%.c,$(BUILD_DIR)/$(1)/%.o,$(wildcard $(APP_DIR)/$(1)/*.c)) \
                 $(patsubst $(APP_DIR)/$(1)/%.S,$(BUILD_DIR)/$(1)/%.o,$(wildcard $(APP_DIR)/$(1)/*.S))

# Shortcut to build a single application only: make <app>
$(1): $(BUILD_DIR)/$(1)/$(1).bin

# Compilation rules for app '$(1)'
$(BUILD_DIR)/$(1)/%.o: $(APP_DIR)/$(1)/%.c
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  CC    $$<"
	@$(CC) $(CFLAGS) $$< -o $$@

$(BUILD_DIR)/$(1)/%.o: $(APP_DIR)/$(1)/%.S
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  CC    $$<"
	@$(CC) $(CFLAGS) $$< -o $$@

# Link rule for app '$(1)'
$(BUILD_DIR)/$(1)/$(1).elf: $$(APP_OBJS_$(1)) $(KERNEL_OBJS)
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  LD    $$<"
	@$(CC) -o $$@ $$^ $(LDFLAGS)

$(BUILD_DIR)/$(1)/$(1).bin: $(BUILD_DIR)/$(1)/$(1).elf
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  OBJCOPY $$<"
	@$(OBJCOPY) -O binary $$< $$@
endef

# instantiate template for each app name (creates explicit targets)
$(foreach a,$(APP_NAMES),$(eval $(call MAKE_APP_RULES,$(a))))

##############################################################################################
# Generic compilation rules (mirror source path under build/)
##############################################################################################
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) $< -o $@

##############################################################################################
# clean
##############################################################################################
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
