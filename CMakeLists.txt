cmake_minimum_required(VERSION 3.1.3)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/firmware/cmake/toolchain-arm-cortex-m.cmake)

project(greatfet)

include(${CMAKE_CURRENT_LIST_DIR}/firmware/cmake/dfu-util.cmake)

# Get version
execute_process(
	COMMAND git log -n 1 --format=%h
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE GIT_VERSION_FOUND
	ERROR_QUIET
	OUTPUT_VARIABLE GIT_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)

if(GIT_VERSION_FOUND)
	set(VERSION "unknown")
else(GIT_VERSION_FOUND)
	set(VERSION ${GIT_VERSION})
endif(GIT_VERSION_FOUND)


# Define board if needed
if(NOT DEFINED BOARD)
	# Default to GreatFET One
	set(BOARD GREATFET_ONE)
endif()

# FIXME: perhaps use a dictionary-pattern?
if(BOARD STREQUAL "GREATFET_ONE")
	set(MCU_PARTNO LPC4330)
endif()

if(BOARD STREQUAL "NXP_XPLORER")
	set(MCU_PARTNO LPC4330)
endif()

if(BOARD STREQUAL "RAD1O_BADGE")
	set(MCU_PARTNO LPC4330)
	set(BUILD_OUTPUT_TYPE "L0ADABLE")
endif()

set(DEFINES_COMMON ${BOARD} LPC43XX ${MCU_PARTNO} VERSION_STRING=\"git-${VERSION}\")


# FIXME: make this configugrable
set(LIBGREAT_PLATFORM lpc43xx)


set(PATH_GREATFET_FIRMWARE ${CMAKE_CURRENT_SOURCE_DIR}/firmware)
set(PATH_GREATFET_FIRMWARE_COMMON ${PATH_GREATFET_FIRMWARE}/common)
set(PATH_LIBGREAT ${CMAKE_CURRENT_SOURCE_DIR}/libgreat)
set(PATH_LIBOPENCM3 ${CMAKE_CURRENT_SOURCE_DIR}/firmware/libopencm3)

set(FLAGS_COMPILE_COMMON -std=gnu99 -Os -g3 -Wall -Wextra -fno-common -MD -fno-builtin-printf -Wno-missing-field-initializers)
set(FLAGS_CPU_COMMON -mthumb)
set(FLAGS_CPU_M0 -mcpu=cortex-m0 -mfloat-abi=soft)
set(FLAGS_CPU_M4 -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)
set(FLAGS_LINK_COMMON -nostartfiles -Wl,--gc-sections)
set(FLAGS_LINK_M0 -TLPC43xx_M0_memory.ld -Tlibopencm3_lpc43xx_m0.ld -Xlinker -Map=m0.map)
set(FLAGS_LINK_M4_COMMON -TLPC4330_M4_memory.ld -TLPC43xx_M4_M0_image_from_text.ld)
set(FLAGS_LINK_M4_SPI	 -Tlibgreat_lpc43xx_rom_to_ram.ld -Xlinker -Map=m4.map)
set(FLAGS_LINK_M4_DFU	 -Tlibgreat_lpc43xx.ld -Xlinker -Map=m4.map)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libgreat)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/firmware)
