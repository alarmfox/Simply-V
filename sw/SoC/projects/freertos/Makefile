# This Makefile orchestrates application building by just invoking app/<app-name/Makefile
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
