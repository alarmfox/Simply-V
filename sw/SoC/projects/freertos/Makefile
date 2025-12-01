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

ifndef ROOT_DIR
$(error Setup script settings.sh has not been sourced, aborting)
endif

APP_DIR := app
APPS    := $(notdir $(wildcard $(APP_DIR)/*))

# default = build all apps
all: $(APPS)

# call make inside app/<name>
$(APPS):
	@$(MAKE) -C $(APP_DIR)/$@ all

# clean everything
clean:
	@for a in $(APPS); do \
		$(MAKE) -C $(APP_DIR)/$$a clean; \
	done

.PHONY: all clean
