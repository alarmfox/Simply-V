###############################################################################
# Toolchain
###############################################################################
CROSS_COMPILE ?= riscv32-unknown-elf-
CC      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy

###############################################################################
# Project structure
###############################################################################
APP_DIR     := app
KERNEL_DIR  := kernel

BUILD_DIR   := build
TARGET_ELF  := $(BUILD_DIR)/freertos_demo.elf
TARGET_BIN  := $(BUILD_DIR)/freertos_demo.bin

###############################################################################
# Source files
###############################################################################
APP_SRCS     := $(wildcard $(APP_DIR)/*.c)
KERNEL_SRCS  := $(wildcard $(KERNEL_DIR)/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/MemMang/heap_1.c)
STARTUP_SRCS := startup.S $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.S)

SRCS := $(APP_SRCS) $(KERNEL_SRCS) $(STARTUP_SRCS)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.S,$(BUILD_DIR)/%.o,$(SRCS)))

###############################################################################
# Includes
###############################################################################
INCLUDES := -I. \
            -I$(KERNEL_DIR)/include \
	    -I$(SW_SOC_ROOT)/lib/uninasoc/inc \
	    -I$(SW_SOC_ROOT)/lib/tinyio/inc \
            -I$(KERNEL_DIR)/portable/GCC/RISC-V \
            -I$(KERNEL_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions

###############################################################################
# Compilation flags
###############################################################################
CFLAGS  := -Wall -Wextra -O0 -ffreestanding -nostdlib -march=rv32ima_zicsr -mabi=ilp32 -g
CFLAGS  += $(INCLUDES)
CFLAGS  += -DportasmHANDLE_INTERRUPT=_ext_handler

LDFLAGS := -Tlinker.ld -lc -lgcc
LDFLAGS += $(SW_SOC_ROOT)/lib/uninasoc/lib/libuninasoc.a $(SW_SOC_ROOT)/lib/tinyio/lib/tinyio.a

###############################################################################
# Default rule
###############################################################################
all: $(TARGET_BIN)

$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@

$(TARGET_ELF): $(OBJS)
	@echo "  LD    $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile .c ->.o
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile .S ->.o
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
