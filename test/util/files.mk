ifndef UTIL_DIR
    $(error UTIL_DIR is not defined)
endif

SRC +=	$(UTIL_DIR)/print.c \
	$(UTIL_DIR)/debug.c \
	$(UTIL_DIR)/xprintf.S \
	$(UTIL_DIR)/timer.c \
	$(UTIL_DIR)/util.c


ifdef CONSOLE_ENABLE
    OPT_DEFS += -DCONSOLE_ENABLE
else
    OPT_DEFS += -DNO_PRINT
    OPT_DEFS += -DNO_DEBUG
endif


# Version string
OPT_DEFS += -DVERSION=$(shell (git describe --always --dirty || echo 'unknown') 2> /dev/null)



#
# LUFA
#
# Path to the LUFA library
LUFA_PATH ?= $(UTIL_DIR)/lufa

# Target architecture (see library "Board Types" documentation).
ARCH = AVR8

# Input clock frequency.
#     This will define a symbol, F_USB, in all source code files equal to the
#     input clock frequency (before any prescaling is performed) in Hz. This value may
#     differ from F_CPU if prescaling is used on the latter, and is required as the
#     raw input clock is fed directly to the PLL sections of the AVR for high speed
#     clock generation for the USB and other AVR subsections. Do NOT tack on a 'UL'
#     at the end, this will be done automatically to create a 32-bit value in your
#     source code.
#
#     If no clock division is performed on the input clock inside the AVR (via the
#     CPU clock adjust registers or the clock division fuses), this will be equal to F_CPU.
F_USB ?= $(F_CPU)


LUFA_OPTS  = -DF_USB=$(F_USB)UL
LUFA_OPTS += -DARCH=ARCH_$(ARCH)
# LUFA library compile-time options and predefined tokens
LUFA_OPTS += -DUSB_DEVICE_ONLY
LUFA_OPTS += -DUSE_FLASH_DESCRIPTORS
LUFA_OPTS += -DUSE_STATIC_OPTIONS="(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)"
LUFA_OPTS += -DFIXED_CONTROL_ENDPOINT_SIZE=8 
LUFA_OPTS += -DFIXED_NUM_CONFIGURATIONS=1
# Interrupt driven control endpoint task(+60)
LUFA_OPTS += -DINTERRUPT_CONTROL_ENDPOINT
# USB Device info
LUFA_OPTS += -DVENDOR_ID=0xFEED
LUFA_OPTS += -DPRODUCT_ID=0xC5C5
LUFA_OPTS += -DDEVICE_VER=0x0001
LUFA_OPTS += -DMANUFACTURER=TMK
LUFA_OPTS += -DPRODUCT=CapSense

OPT_DEFS += $(LUFA_OPTS)


# Create the LUFA source path variables by including the LUFA makefile
LUFA_ROOT_PATH = $(LUFA_PATH)/LUFA
include $(LUFA_PATH)/LUFA/Build/lufa_sources.mk 

LUFA_SRC = $(UTIL_DIR)/lufa.c \
	   $(UTIL_DIR)/descriptor.c \
	   $(LUFA_SRC_USB)

SRC += $(LUFA_SRC)


# Search Path
#VPATH += $(UTIL_DIR)
VPATH += $(LUFA_PATH)
