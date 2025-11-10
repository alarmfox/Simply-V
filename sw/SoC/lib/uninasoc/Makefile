## UNINASOC
UNINASOC_DIR = $(SW_SOC_ROOT)/lib/uninasoc
UNINASOC_SRCS := $(wildcard $(UNINASOC_DIR)/src/*.c)
UNINASOC_OBJS := $(patsubst $(UNINASOC_DIR)/src/%.c, %.o, $(wildcard $(UNINASOC_DIR)/src/*.c))
UNINASOC_SRCS_DIR := $(UNINASOC_DIR)/src
UNINASOC_LIB_DIR := $(UNINASOC_DIR)/lib
UNINASOC_LIB := $(UNINASOC_LIB_DIR)/libuninasoc.a
UNINASOC_INC := $(UNINASOC_DIR)/inc
TINY_IO_INC := $(SW_SOC_ROOT)/lib/tinyio/inc
TINY_IO_LIB := $(SW_SOC_ROOT)/lib/tinyio/lib/tinyio.a

include $(SW_ROOT)/SoC/common/config.mk

all: $(UNINASOC_LIB)

# Build sources for devices
%.o: $(UNINASOC_SRCS_DIR)/%.c
	@echo "[CC] $<"
	$(CC) -c $< -o $@ -I$(UNINASOC_INC) -I$(TINY_IO_INC) $(CFLAGS) $(MACRO_LIST)

# Build libuninasoc.a library
$(UNINASOC_LIB): $(UNINASOC_OBJS)
	@mkdir -p $(UNINASOC_LIB_DIR)
	@echo "CREATE $@"> mri.tmp
	@echo "ADDLIB $(TINY_IO_LIB)">> mri.tmp
	@printf "ADDMOD %s\n" $(UNINASOC_OBJS) >> mri.tmp
	@echo "SAVE" >> mri.tmp
	@echo "END" >> mri.tmp
	$(AR) -M < mri.tmp
	@rm mri.tmp
	@rm $(UNINASOC_OBJS)

dump: $(UNINASOC_LIB)
	$(OBJDUMP) -d $^ > lib/libuninasoc.dump

clean:
	@rm -rf $(UNINASOC_LIB_DIR)
