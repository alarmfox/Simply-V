## UNINASOC
UNINASOC_DIR = $(SW_SOC_ROOT)/lib/uninasoc
UNINASOC_SRCS := $(wildcard $(UNINASOC_DIR)/src/*.c)
UNINASOC_OBJS := $(patsubst $(UNINASOC_DIR)/src/%.c, %.o, $(wildcard $(UNINASOC_DIR)/src/*.c))
UNINASOC_SRCS_DIR := $(UNINASOC_DIR)/src
UNINASOC_LIB_DIR := $(UNINASOC_DIR)/lib
UNINASOC_LIB := $(UNINASOC_LIB_DIR)/libuninasoc.a
UNINASOC_INC := $(UNINASOC_DIR)/inc
TINY_IO_INC := $(SW_SOC_ROOT)/lib/tinyio/inc
TINY_IO_LIB := $(wildcard $(SW_SOC_ROOT)/lib/tinyio/lib/tinyio.a)
TINY_IO_OBJ_DIR := $(SW_SOC_ROOT)/lib/uninasoc/lib/tinyio_obj
TINY_IO_OBJS := $(wildcard $(TINY_IO_OBJ_DIR)/*.o)

include $(SW_ROOT)/SoC/common/config.mk

all: $(UNINASOC_LIB)

# Build sources for devices
%.o: $(UNINASOC_SRCS_DIR)/%.c
	@echo "[CC] $<"
	$(CC) -c $< -o $@ -I$(UNINASOC_INC) -I$(TINY_IO_INC) $(CFLAGS) $(MACRO_LIST)

# Build libuninasoc.a library
$(UNINASOC_LIB): $(UNINASOC_OBJS)
	@mkdir -p $(UNINASOC_LIB_DIR) $(TINY_IO_OBJ_DIR)
	@echo "[OBJ] Extracting tinyio object files"
	$(AR) -x $(TINY_IO_LIB) --output $(TINY_IO_OBJ_DIR)
	@echo "[OBJ] Creating libuninasoc.a library"
	$(AR) rcs $(UNINASOC_LIB) $(UNINASOC_OBJS)  $(wildcard $(TINY_IO_OBJ_DIR)/*.o)
	@rm -f $(UNINASOC_OBJS)

dump: $(UNINASOC_LIB)
	$(OBJDUMP) -d $^ > lib/libuninasoc.dump

clean:
	@rm -rf $(UNINASOC_LIB_DIR)
