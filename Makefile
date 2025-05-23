PROJECT_NAME    := Template
TARGET       	:= nrf52840
BUILD_DIR 		:= build

SDK_ROOT := components/nRF5_SDK
SDK_CONFIG_FILE := main/config/sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
LINKER_SCRIPT  := partitions.ld

VERBOSE ?= 0
ifeq ($(VERBOSE),1)
  NO_ECHO :=
else
  NO_ECHO := @
endif

# Makefile
-include config.mk

# Source files common to all targets
ASMM_SOURCES = $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S

SRC_FILES += \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_rtt.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
	$(SDK_ROOT)/components/libraries/util/app_error.c \
	$(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
	$(SDK_ROOT)/components/libraries/util/app_error_weak.c \
	$(SDK_ROOT)/components/libraries/util/app_util_platform.c \
	$(SDK_ROOT)/components/libraries/util/nrf_assert.c \
	$(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
	$(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
	$(SDK_ROOT)/components/libraries/atomic_flags/nrf_atflags.c \
	$(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
	$(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
	$(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
	$(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
	$(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
	$(SDK_ROOT)/components/libraries/sortlist/nrf_sortlist.c \
	$(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
	$(SDK_ROOT)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c \
	$(SDK_ROOT)/components/libraries/usbd/app_usbd_string_desc.c \
	$(SDK_ROOT)/components/libraries/usbd/app_usbd_serial_num.c \
	$(SDK_ROOT)/components/libraries/usbd/app_usbd_core.c \
	$(SDK_ROOT)/components/libraries/usbd/app_usbd.c \
	$(SDK_ROOT)/components/libraries/cli/cdc_acm/nrf_cli_cdc_acm.c \
	$(SDK_ROOT)/components/libraries/cli/nrf_cli.c \
	$(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
	$(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
	$(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
	$(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_power.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_power.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_usbd.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_timer.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rtc.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
	$(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
	$(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
	$(SDK_ROOT)/components/ble/common/ble_advdata.c \
	$(SDK_ROOT)/components/ble/common/ble_srv_common.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_params.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_state.c \
	$(SDK_ROOT)/components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c \
	$(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
	$(SDK_ROOT)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
	$(SDK_ROOT)/components/ble/nrf_ble_gq/nrf_ble_gq.c\
	$(SDK_ROOT)/components/ble/nrf_ble_scan/nrf_ble_scan.c \
	$(SDK_ROOT)/components/ble/ble_db_discovery/ble_db_discovery.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \


# Include folders common to all targets
INC_FOLDERS += \
	-Imain/config \
	-I$(SDK_ROOT)/modules/nrfx/hal \
	-I$(SDK_ROOT)/modules/nrfx/drivers/include \
	-I$(SDK_ROOT)/modules/nrfx \
	-I$(SDK_ROOT)/modules/nrfx/mdk \
	-I$(SDK_ROOT)/integration/nrfx \
	-I$(SDK_ROOT)/integration/nrfx/legacy \
	-I$(SDK_ROOT)/components \
	-I$(SDK_ROOT)/components/toolchain/cmsis/include \
	-I$(SDK_ROOT)/components/libraries/log \
	-I$(SDK_ROOT)/components/libraries/experimental_section_vars \
	-I$(SDK_ROOT)/components/libraries/fifo \
	-I$(SDK_ROOT)/components/libraries/atomic_fifo \
	-I$(SDK_ROOT)/components/libraries/log/src \
	-I$(SDK_ROOT)/components/libraries/ringbuf \
	-I$(SDK_ROOT)/components/libraries/delay \
	-I$(SDK_ROOT)/components/libraries/memobj \
	-I$(SDK_ROOT)/components/libraries/atomic \
	-I$(SDK_ROOT)/components/libraries/crc16 \
	-I$(SDK_ROOT)/components/libraries/util \
	-I$(SDK_ROOT)/components/libraries/queue \
	-I$(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \
	-I$(SDK_ROOT)/components/libraries/usbd/class/cdc \
	-I$(SDK_ROOT)/components/libraries/usbd \
	-I$(SDK_ROOT)/components/libraries/cli/cdc_acm \
	-I$(SDK_ROOT)/components/libraries/cli \
	-I$(SDK_ROOT)/components/libraries/balloc \
	-I$(SDK_ROOT)/components/libraries/sortlist \
	-I$(SDK_ROOT)/components/libraries/strerror \
	-I$(SDK_ROOT)/components/libraries/atomic_flags \
	-I$(SDK_ROOT)/components/softdevice/s140/headers/nrf52 \
	-I$(SDK_ROOT)/components/softdevice/s140/headers \
	-I$(SDK_ROOT)/components/softdevice/common \
	-I$(SDK_ROOT)/components/ble/ble_advertising \
	-I$(SDK_ROOT)/components/ble/ble_dtm \
	-I$(SDK_ROOT)/components/ble/common \
	-I$(SDK_ROOT)/components/ble/ble_link_ctx_manager \
	-I$(SDK_ROOT)/components/ble/nrf_ble_gatt \
	-I$(SDK_ROOT)/components/ble/nrf_ble_qwr \
	-I$(SDK_ROOT)/components/ble/ble_racp \
	-I$(SDK_ROOT)/components/ble/ble_db_discovery \
	-I$(SDK_ROOT)/components/ble/nrf_ble_gq \
	-I$(SDK_ROOT)/components/ble/nrf_ble_scan \
	-I$(SDK_ROOT)/external/segger_rtt \
	-I$(SDK_ROOT)/external/fprintf \
	-I$(SDK_ROOT)/external/utf_converter \

CPP_FILES += \
	main/driver/timer.cpp \
	main/driver/pwm.cpp \
	main/periph/led.cpp \
	main/comm/gatt_client.cpp \
	main/comm/dongle.cpp \
	main/comm/usb_cli.cpp \
	main/misc/cmds.cpp \
	main/main.cpp \

INC_FOLDERS += \
	-Imain/driver/include \
	-Imain/periph/include \
	-Imain/comm/include \

# Libraries common to all targets
LIB_FILES += -lc -lnosys -lm


#######################################
# binaries
#######################################

PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CXX = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# float-abi
FLOAT-ABI = -mfloat-abi=hard -mfpu=fpv4-sp-d16

# mcu
MCU = $(CPU) -mthumb -mabi=aapcs $(FLOAT-ABI)
# macros for gcc
# AS defines
AS_DEFS = -x assembler-with-cpp

# C defines
C_DEFS =  \
	-DCONFIG_GPIO_AS_PINRESET \
	-DFLOAT_ABI_HARD \
	-DNRF52840_XXAA \
	-DNRF_SD_BLE_API_VERSION=7 \
	-DS140 \
	-DSOFTDEVICE_PRESENT \
	-DDEBUG \
	-DDEBUG_NRF_USER

# Optimization flags
OPT = -O0 -g3
# Uncomment the line below to enable link time optimization
# OPT += -flto

# C flags common to all targets
CFLAGS = $(MCU) $(C_DEFS) $(INC_FOLDERS) $(OPT)
CFLAGS += -Wall -Wuninitialized
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums
CFLAGS += -Wno-error=array-bounds
CFLAGS += -Wunused-function
CFLAGS += -fdiagnostics-color=always
CFLAGS += -MMD -MP

# C++ flags common to all targets
CXXFLAGS = $(CFLAGS) -std=c++17
# Assembler flags common to all targets
ASMFLAGS = $(MCU) $(AS_DEFS) $(OPT)
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -D__HEAP_SIZE=8192
ASMFLAGS += -D__STACK_SIZE=8192
ASMFLAGS += -MMD -MP

# Linker flags
LDFLAGS = $(MCU) -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

LIB_FILES += -lstdc++



# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_FILES:.c=.o)))
vpath %.c $(sort $(dir $(SRC_FILES)))
# list of cpp objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_FILES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

# depend file list
DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "\033[32m[CC]  Compiling: $< -> $@\033[0m"
	$(NO_ECHO)$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	@echo "\033[34m[CXX] Compiling: $< -> $@\033[0m"
	$(NO_ECHO)$(CXX) -c $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "\033[32m[AS]  Compiling: $< -> $@\033[0m"
	$(NO_ECHO)$(AS) -c $(ASMFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	@echo "\033[32m[AS]  Compiling: $< -> $@\033[0m"
	$(NO_ECHO)$(AS) -c $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "\033[32m[LD]  Linking: $(TARGET).elf\033[0m"
	$(NO_ECHO)$(CXX) $(OBJECTS) $(LDFLAGS) $(LIB_FILES) -o $@
	@echo "\033[36m[SZ]  Size:\033[0m"
	$(NO_ECHO)$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "\033[36m[HEX]  Build: $< -> $@\033[0m"
	$(NO_ECHO)$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "\033[36m[BIN]  Build: $< -> $@\033[0m"
	$(NO_ECHO)$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir -p $@

.PHONY: all clean flash flash_softdevice sdk_config help

# clean up
clean:
	-rm -r $(BUILD_DIR)
# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52840
	@echo		flash_softdevice
	@echo		sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

ifdef JLINK_PATH
JLINK = $(JLINK_PATH)/JLinkExe
else
JLINK = JLinkExe
endif
JLINK_DEVICE = NRF52840_XXAA
JLINK_IF = SWD
JLINK_SPEED = 4000
JLINK_SCRIPT = jlink_flash.jlink
JLINK_SOFTDEVICE_SCRIPT = jlink_flash_softdevice.jlink

SOFTDEVICE_HEX = $(SDK_ROOT)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex

# 生成 J-Link 烧录脚本
$(JLINK_SCRIPT):
	@echo "Device $(JLINK_DEVICE)" > $(JLINK_SCRIPT)
	@echo "Speed $(JLINK_SPEED)" >> $(JLINK_SCRIPT)
	@echo "SelectInterface $(JLINK_IF)" >> $(JLINK_SCRIPT)
	@echo "Connect" >> $(JLINK_SCRIPT)
	@echo "Loadfile $(BUILD_DIR)/$(TARGET).hex" >> $(JLINK_SCRIPT)
	@echo "Reset" >> $(JLINK_SCRIPT)
	@echo "Exit" >> $(JLINK_SCRIPT)

# 生成 J-Link 协议栈烧录脚本
$(JLINK_SOFTDEVICE_SCRIPT):
	@echo "Device $(JLINK_DEVICE)" > $(JLINK_SOFTDEVICE_SCRIPT)
	@echo "Speed $(JLINK_SPEED)" >> $(JLINK_SOFTDEVICE_SCRIPT)
	@echo "SelectInterface $(JLINK_IF)" >> $(JLINK_SOFTDEVICE_SCRIPT)
	@echo "Connect" >> $(JLINK_SOFTDEVICE_SCRIPT)
	@echo "Loadfile $(SOFTDEVICE_HEX)" >> $(JLINK_SOFTDEVICE_SCRIPT)
	@echo "Reset" >> $(JLINK_SOFTDEVICE_SCRIPT)
	@echo "Exit" >> $(JLINK_SOFTDEVICE_SCRIPT)

# 烧录到设备
flash: $(BUILD_DIR)/$(TARGET).hex $(JLINK_SCRIPT)
	$(JLINK) $(JLINK_SCRIPT)

# 烧录软设备
flash_softdevice: $(SOFTDEVICE_HEX) $(JLINK_SOFTDEVICE_SCRIPT)
	$(JLINK) $(JLINK_SOFTDEVICE_SCRIPT)

sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
