# Author: Manuel Maddaluno <manuel.maddaluno@unina.it>
# Description: utility functions to write RTL file for CLOCKS declaration and assignments

####################
# Import libraries #
####################
# Parse args
import sys

# Get env vars
import os

# String templating
from jinja2 import Template

# Sub-scripts
import configuration
import utils as ut


# RTL files to edit
RTL_FILES = {
    "UNINASOC": f"{os.environ.get('XILINX_ROOT')}/rtl/uninasoc_clk_assignments.svinc",
}

template_str = r"""// This file is auto-generated with {{ current_file_path }}

/////////////////////////////////////////
// Clocks declaration and assignments  //
/////////////////////////////////////////
assign main_clk = clk_{{ main_clock_domain }}MHz;
assign main_rstn = rstn_{{ main_clock_domain }}MHz;
logic clk_300MHz;
logic rstn_300MHz;

{% for clock_domain in clock_domains -%}
logic {{ clock_domain.name }}_clk;
assign {{ clock_domain.name }}_clk = clk_{{ clock_domain.clock }}MHz;
logic {{ clock_domain.name }}_rstn;
assign {{ clock_domain.name }}_rstn = rstn_{{ clock_domain.clock }}MHz;
{% endfor %}
logic HBUS_clk;
"""


def declare_and_assign_clocks_rtl(config: configuration.Configuration) -> list:
    # Get (name: clock) data structure
    clock_domains = []
    # Navigate the 2 lists. Skip HBUS or DDR4CH* because they have their clock
    for clock, name in zip(mbus_config.RANGE_CLOCK_DOMAINS, mbus_config.RANGE_NAMES):
        if name == "HBUS" or name.startswith("DDR4CH"):
            continue
        clock_domains.append(
            {
                "name": name,
                "clock": clock,
            }
        )
    return clock_domains


########
# MAIN #
########
if __name__ == "__main__":
    config_file_names = sys.argv[1:]
    configs = ut.read_config(config_file_names)
    mbus_config: configuration.Configuration = None

    # Get the MBUS configuration
    for config in configs:
        if config.CONFIG_NAME == "MBUS":
            mbus_config = config
            break

    if mbus_config is None:
        sys.exit(0)

    # Get clock domains
    clock_domains = declare_and_assign_clocks_rtl(mbus_config)

    template = Template(template_str)

    rendered = template.render(
        current_file_path=os.path.basename(__file__),
        clock_domains=clock_domains,
        main_clock_domain=config.MAIN_CLOCK_DOMAIN,
    )

    with open(RTL_FILES["UNINASOC"], "w") as f:
        f.write(rendered)
