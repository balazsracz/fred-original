# Path to the MPASM assembler directory. Can be overridden on command line.
# Note: If compilation fails because of missing uppercase header files,
# you must run the 'createbackcompatiblelinks' script in the mpasmx directory:
#   cd $(MPASM_DIR) && ./createbackcompatiblelinks
MPASM_DIR ?= /home/bracz/old/opt/microchip/mplabx.1.85/mpasmx

# Assembler executable
MPASM = $(MPASM_DIR)/mpasmx

# Command line switches (-q for quiet mode, replacing Windows /q)
MPASM_SW = -q

# Targets to build
TARGETS = fred4k_f.HEX fred_f.HEX fred_c.HEX ftest_f.HEX ftest_c.HEX

# Include dependencies
DEPS = $(wildcard *.inc) $(wildcard *.INC)

# PICkit 2 programming settings
DEVICE ?= PIC16F84A
READOUT ?= readout.hex
HEX ?= fred_f.HEX

.PHONY: all clean distclean check_symlinks read_flash upgrade_flash

all: check_symlinks $(TARGETS)

check_symlinks:
	@if [ ! -f "$(MPASM_DIR)/P16F84.INC" ]; then \
		echo "ERROR: Backward compatibility symlinks do not exist in $(MPASM_DIR)."; \
		echo "Please run: cd $(MPASM_DIR) && ./createbackcompatiblelinks"; \
		exit 1; \
	fi

%.HEX: %.asm $(DEPS) | check_symlinks
	$(MPASM) $(MPASM_SW) $<

clean:
	rm -f *.COD *.ERR *.LST *.O *.cod *.err *.lst *.o ___*.fil ___*.FIL *.HEX *.hex

distclean: clean

read_flash:
	pk2cmd -P$(DEVICE) -GF$(READOUT)

flash:
	pk2cmd -P$(DEVICE) -E -M -F$(HEX)

upgrade_flash:
	pk2cmd -P$(DEVICE) -E -MP -Z -F$(HEX)

restore_eeprom:
	pk2cmd -P$(DEVICE) -ME -F readout.hex
