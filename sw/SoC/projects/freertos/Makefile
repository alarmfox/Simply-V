###############################################################################
# Toolchain
###############################################################################
XLEN          ?= 32
RV_PREFIX     = riscv$(XLEN)-unknown-elf-
CC            := $(RV_PREFIX)gcc
OBJCOPY       := $(RV_PREFIX)objcopy

###############################################################################
# Project structure
###############################################################################
APP_DIR     := app
KERNEL_DIR  := FreeRTOS-Kernel

BUILD_DIR   := build

###############################################################################
# Kernel sources (shared)
###############################################################################
KERNEL_SRCS  := $(wildcard $(KERNEL_DIR)/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/MemMang/heap_1.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.S)

KERNEL_OBJS := $(BUILD_DIR)/startup.o $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.S,$(BUILD_DIR)/%.o,$(KERNEL_SRCS)))

###############################################################################
# CFLAGS
###############################################################################
INCLUDES := -I. \
            -I$(KERNEL_DIR)/include \
            -I$(KERNEL_DIR)/portable/GCC/RISC-V \
            -I$(KERNEL_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions

ARCH    := rv$(XLEN)ima
ARCH    := $(addsuffix _zicsr_zifencei, $(ARCH))

ifeq ($(XLEN), 64)
ABI     := lp64
else ifeq ($(XLEN), 32)
ABI     := ilp32
else
$(error Unsupported XLEN value: $(XLEN))
endif

CFLAGS  := -march=$(ARCH) -mabi=$(ABI)
CFLAGS  += -Wall -Wextra -O0 -ffreestanding -nostdlib

INCLUDES += -I$(SW_SOC_ROOT)/lib/uninasoc/inc \
            -I$(SW_SOC_ROOT)/lib/tinyio/inc

ifeq ($(DEBUG), 1)
CFLAGS  += -g
endif

CFLAGS  += $(INCLUDES)
###############################################################################
# Linker flags
###############################################################################
LDFLAGS := -Tlinker.ld -lc -lgcc
LDFLAGS += $(SW_SOC_ROOT)/lib/uninasoc/lib/libuninasoc.a $(SW_SOC_ROOT)/lib/tinyio/lib/tinyio.a

###############################################################################
# Discover applications
###############################################################################
APP_DIRS := $(wildcard $(APP_DIR)/*)     # app/basic app/other ...
APP_NAMES := $(notdir $(APP_DIRS))       # basic other ...

###############################################################################
# Build targets (one explicit pair of targets per app)
###############################################################################
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

# Compilation rules for app '$(1)'
$(BUILD_DIR)/$(1)/%.o: $(APP_DIR)/$(1)/%.c
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  CC    $$<"
	@$(CC) $(CFLAGS) -c $$< -o $$@

$(BUILD_DIR)/$(1)/%.o: $(APP_DIR)/$(1)/%.S
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  CC    $$<"
	@$(CC) $(CFLAGS) -c $$< -o $$@

# Link rule for app '$(1)'
$(BUILD_DIR)/$(1)/$(1).elf: $$(APP_OBJS_$(1)) $(KERNEL_OBJS)
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  LD    $$<"
	@$(CC) $(CFLAGS) -o $$@ $$^ $(LDFLAGS)

$(BUILD_DIR)/$(1)/$(1).bin: $(BUILD_DIR)/$(1)/$(1).elf
	@mkdir -p $(BUILD_DIR)/$(1)
	@echo "  OBJCOPY $$<"
	@$(OBJCOPY) -O binary $$< $$@
endef

# instantiate template for each app name (creates explicit targets)
$(foreach a,$(APP_NAMES),$(eval $(call MAKE_APP_RULES,$(a))))

###############################################################################
# Generic compilation rules (mirror source path under build/)
###############################################################################
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

###############################################################################
# clean
###############################################################################
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
