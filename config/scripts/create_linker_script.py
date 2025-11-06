#!/bin/python3.10
# Author: Stefano Toscano 		<stefa.toscano@studenti.unina.it>
# Author: Vincenzo Maisto 		<vincenzo.maisto2@unina.it>
# Author: Stefano Mercogliano 		<stefano.mercogliano@unina.it>
# Description:
#   Generate a linker script file from the CSV configuration.
# Note:
#   Addresses overlaps are not sanitized.
# Args:
#   1: Input configuration file
#   2: Output generated ld script

####################
# Import libraries #
####################
# Parse args
import sys
# For basename
import os
# Manipulate CSV
import pandas as pd
# String templating
from jinja2 import Template

##############
# Parse args #
##############

# CSV configuration file path
config_file_names = [
		'config/configs/embedded/config_main_bus.csv',
		'config/configs/embedded/config_peripheral_bus.csv',
		'config/configs/embedded/config_highperformance_bus.csv',
	]

if len(sys.argv) >= len(config_file_names)+1:
	# Get the array of bus names from the second arg to the last but one
	config_file_names = sys.argv[1:len(config_file_names)+1]

# Target linker script file
ld_file_name = 'sw/SoC/common/UninaSoC.ld'
if len(sys.argv) >= 5:
	# Get the linker script name, the last arg
	ld_file_name = sys.argv[len(config_file_names)+1]


###############
# Read config #
###############
# Read CSV files for each bus
config_dfs = []
for name in config_file_names:
	config_dfs.append(pd.read_csv(name, sep=",", index_col=0))

# Each bus has an element in these vectors
NUM_MI = []
RANGE_NAMES = []
RANGE_BASE_ADDR = []
RANGE_ADDR_WIDTH = []
# For each bus
for config_df in config_dfs:

	# Skip DISABLE buses
	if config_df.loc["PROTOCOL"]["Value"] == "DISABLE":
		continue

	# Read number of masters interfaces
	NUM_MI.append(int(config_df.loc["NUM_MI"]["Value"]))
	# print("[DEBUG] NUM_MI", NUM_MI)

	# Read slaves' names
	RANGE_NAMES.append(config_df.loc["RANGE_NAMES"]["Value"].split())
	# print("[DEBUG] RANGE_NAMES", RANGE_NAMES)

	# Read address Ranges
	RANGE_BASE_ADDR.append(config_df.loc["RANGE_BASE_ADDR"]["Value"].split())
	# print("[DEBUG] RANGE_BASE_ADDR", RANGE_BASE_ADDR)

	# Read address widths
	RANGE_ADDR_WIDTH.append(config_df.loc["RANGE_ADDR_WIDTH"]["Value"].split())
	# Turns the values into Integers
	for i in range(len(RANGE_ADDR_WIDTH[-1])):
		RANGE_ADDR_WIDTH[-1][i] = int(RANGE_ADDR_WIDTH[-1][i])

# Currently the first memory device is selected as the boot memory device
BOOT_MEMORY_BLOCK = 0x0


# ################
# # Sanity check #
# ################
# For each bus
for i in range(len(NUM_MI)):
	assert (NUM_MI[i] == len(RANGE_NAMES[i])) & (NUM_MI[i] == len(RANGE_BASE_ADDR[i]) ) & (NUM_MI[i]  == len(RANGE_ADDR_WIDTH[i])), \
		"Mismatch in lenght of configurations: NUM_MI(" + str(NUM_MI[i]) + "), RANGE_NAMES (" + str(len(RANGE_NAMES[i])) + \
		"), RANGE_BASE_ADDR(" + str(len(RANGE_BASE_ADDR[i])) + ") RANGE_ADDR_WIDTH(" + str(len(RANGE_ADDR_WIDTH[i])) + ")"

##########################
# Generate memory blocks #
##########################
# Currently only one copy of BRAM, DDR and HBM memory ranges are supported.

device_dict = {
	'memory':			[],
	'peripheral':		[],
}

counter = 0
# For each bus
for i in range(len(RANGE_NAMES)):
	for device in RANGE_NAMES[i]:
		match device:
			# memory blocks
			case "BRAM" | "DDR" | "HBM":
				# TODO: tailor each memory block with specific permissions. Eg. BRAM (rx)
				device_dict['memory'].append({'device': device, 'permissions': 'xrw', 'base': int(RANGE_BASE_ADDR[i][counter], 16), 'range': 1 << RANGE_ADDR_WIDTH[i][counter]})

			# peripherals
			case _:
				# Check if the device is not a bus (the last three chars are not BUS)
				if device[-3:] != "BUS":
					device_dict['peripheral'].append({'device': device, 'base': int(RANGE_BASE_ADDR[i][counter], 16), 'range': 1 << RANGE_ADDR_WIDTH[i][counter]})

		# Increment counter
		counter += 1
		# If we reach the last element of a bus we need to reset the counter to start with a new bus
		if counter == len(RANGE_NAMES[i]):
			counter = 0

device_dict['global_symbols'] = [
		("_stack_start", device_dict['memory'][BOOT_MEMORY_BLOCK]['base'] + device_dict['memory'][BOOT_MEMORY_BLOCK]['range'] - 0x10),
		("_vector_table_start", device_dict['memory'][BOOT_MEMORY_BLOCK]['base']),
		("_vector_table_end", device_dict['memory'][BOOT_MEMORY_BLOCK]['base'] + 32 * 4)
		]

###############################
# Generate Linker Script File #
###############################

template_str = r""" /* This file is auto-dgenerated with {{ current_file_path }} */
MEMORY
{
{% set indent = ' ' * 4 %}
{% for block in memory -%}
{{ indent }}{{ block.device }} ({{ block.permissions }}): ORIGIN = {{ block.base }}, LENGTH = 0x{{ "%x"|format(block.range) }}
{% endfor %}
}

/* Peripherals symbols */
{% for peripheral in peripherals -%}
_peripheral_{{ peripheral.device }}_start = 0x{{ "%016x"|format(peripheral.base) }};
_peripheral_{{ peripheral.device }}_end   = 0x{{ "%016x"|format(peripheral.base + peripheral.range) }};
{% endfor %}

/* Global symbols */
{% for name, value in global_symbols -%}
{{ name }} = 0x{{ "%016x"|format(value) }};
{% endfor %}

/* Sections */
SECTIONS
{
    .vector_table _vector_table_start :
    {
        KEEP(*(.vector_table))
    }> {{ initial_memory_name }}

    .text :
    {
       . = ALIGN(32);
       _text_start = .;
       *(.text.handlers)
       *(.text.start)
       *(.text)
       *(.text*)
       . = ALIGN(32);
       _text_end = .;
    }> {{ initial_memory_name }}
}
"""
if __name__ == "__main__":

	template = Template(template_str)

	rendered = template.render(
		current_file_path=os.path.basename(__file__),
		peripherals=device_dict['peripheral'],
		memory=device_dict['memory'],
		global_symbols=device_dict['global_symbols'],
		initial_memory_name=device_dict['memory'][BOOT_MEMORY_BLOCK]['device'],
	)

	# === Output to file ===
	with open(ld_file_name, "w") as f:
		f.write(rendered)
