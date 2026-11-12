USE_LIBA = 0
ION_KEYBOARD_LAYOUT = layout_B2
EPSILON_GETOPT = 1

SFLAGS += -fPIE

ifeq ($(strip $(TARGET)),)
  ifneq ($(filter release,$(MAKECMDGOALS)),)
    $(error TARGET must be set explicitly for release builds)
  endif
  TARGET := $(HOST)
  $(warning TARGET is not set; defaulting to $(TARGET). Set TARGET explicitly for reproducible builds.)
endif

BUILD_DIR := $(BUILD_DIR)/$(TARGET)

include build/platform.simulator.$(TARGET).mak
