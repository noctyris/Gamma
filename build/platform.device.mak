ifeq ($(strip $(MODEL)),)
  ifneq ($(filter release,$(MAKECMDGOALS)),)
    $(error MODEL must be set explicitly for release builds)
  endif
  MODEL := bootloader
  $(warning MODEL is not set; defaulting to bootloader. Set MODEL explicitly for reproducible builds.)
endif

USE_LIBA = 1
EXE = elf

EPSILON_TELEMETRY ?= 0

BUILD_DIR := $(BUILD_DIR)/$(MODEL)

$(BUILD_DIR)/python/port/port.o: CXXFLAGS += -DMP_PORT_USE_STACK_SYMBOLS=1

include build/platform.device.$(MODEL).mak
