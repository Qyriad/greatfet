#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

#set(CMAKE_TOOLCHAIN_FILE ../firmware/cmake/toolchain-arm-cortex-m.cmake)

project(libgreat C CXX ASM)

set(LD_SCRIPT "-T${CMAKE_CURRENT_SOURCE_DIR}/../firmware/common/LPC43xx_M4_memory.ld -Tlibopencm3_lpc43xx_rom_to_ram.ld -T${CMAKE_CURRENT_SOURCE_DIR}/..firmware/common/LPC43xx_M4_M0_image_from_text.ld")

set(LD_SCRIPT_DFU "-T${CMAKE_CURRENT_SOURCE_DIR}/../firmware/common/LPC4330_M4.memory.ld -Tlibopencm3_lpc43xx.ld -T${CMAKE_CURRENT_SOURCE_DIR}/../firmware/common/LPC43xx_M4_M0_image_from_text.ld")

add_library(libgreat STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/platform/lpc43xx/crt0.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/platform/lpc43xx/sync.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/platform/lpc43xx/sync.S
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/memory/allocator.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/memory/allocator/umm_malloc.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/lpc43xx/usb.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/lpc43xx/usb_host.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/lpc43xx/usb_request.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/lpc43xx/usb_standard_request.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/lpc43xx/usb_queue.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/lpc43xx/usb_queue_host.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/usb/comms_backend.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/gpio/lpc43xx/gpio.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/comms/comms_class.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/drivers/comms/utils.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/classes/core.c
	${CMAKE_CURRENT_SOURCE_DIR}/firmware/classes/firmware.c
	)

# because we don't want a file called liblibgreat.a
set_target_properties(libgreat PROPERTIES PREFIX "")

target_compile_definitions(libgreat PUBLIC LPC43XX_M4)

target_include_directories(libgreat SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/libopencm3/include)
target_include_directories(libgreat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/firmware/include ${CMAKE_CURRENT_SOURCE_DIR}/firmware/include/platform/lpc43xx ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/common)

target_compile_options(libgreat PRIVATE -Os -g3 -Wall -Wextra -fno-common -fno-builtin-printf -Wmissing-field-initializers -nostartfiles -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)

target_link_libraries(libgreat c nosys)
