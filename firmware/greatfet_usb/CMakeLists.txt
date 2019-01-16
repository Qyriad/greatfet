#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

project(greatfet_usb C CXX ASM)

# -------M0 Stuff-------

set(SOURCE_M0 ${PATH_GREATFET_FIRMWARE_COMMON}/m0_sleep.c)

configure_file(${PATH_GREATFET_FIRMWARE}/cmake/m0_bin.s.cmake m0_bin.s)

add_executable(greatfet_usb_m0.elf ${SOURCE_M0})

target_compile_options(greatfet_usb_m0.elf PRIVATE ${FLAGS_COMPILE_COMMON} ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M0})
target_compile_definitions(greatfet_usb_m0.elf PRIVATE ${DEFINES_COMMON})
target_link_options(greatfet_usb_m0.elf PRIVATE ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M0} ${FLAGS_LINK_COMMON} ${FLAGS_LINK_M0})

target_link_directories(greatfet_usb_m0.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/lpc43xx/linker)

target_link_libraries(greatfet_usb_m0.elf PRIVATE c nosys opencm3_lpc43xx_m0)

# ELF -> bin
add_custom_target(greatfet_usb_m0.bin ALL DEPENDS greatfet_usb_m0.elf COMMAND ${CMAKE_OBJCOPY} -Obinary greatfet_usb_m0.elf greatfet_usb_m0.bin)


# -------M4 Stuff-------

# firmware/common sources files used by greatfet_usb
set(SOURCE_COMMON_M4
	${PATH_GREATFET_FIRMWARE_COMMON}/gpdma.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_dma.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_scu.c
	${PATH_GREATFET_FIRMWARE_COMMON}/glitchkit.c
	${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c
	${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c
	${PATH_GREATFET_FIRMWARE_COMMON}/debug.c
	${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c
	${PATH_GREATFET_FIRMWARE_COMMON}/one_wire.c
	${PATH_GREATFET_FIRMWARE_COMMON}/greatfet_core.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spiflash_target.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spiflash.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spi_ssp.c
	${PATH_GREATFET_FIRMWARE_COMMON}/i2c_bus.c
	${PATH_GREATFET_FIRMWARE_COMMON}/i2c_lpc.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_lpc.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_int.c
	${PATH_GREATFET_FIRMWARE_COMMON}/printf.c
	${PATH_GREATFET_FIRMWARE_COMMON}/time.c
	)

# AUX SOURCE DIRECTORY CLASSES
message(STATUS "Reading classes from ${CMAKE_CURRENT_SOURCE_DIR}/classes")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/classes CLASSES_SOURCES)
message(STATUS "Using classes: ${CLASSES_SOURCES}")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/glitchkit CLASSES_GLITCHKIT_SOURCES)

# Common object files that will be linked into both SPI and DFU binaries
add_library(obj_greatfet_usb OBJECT
	${CMAKE_CURRENT_SOURCE_DIR}/greatfet_usb.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_request_handlers.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_bulk_buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/pin_manager.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_descriptor.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_device.c
	${CMAKE_CURRENT_SOURCE_DIR}/usb_endpoint.c
	${CMAKE_CURRENT_SOURCE_DIR}/sgpio_isr.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_spi.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_adc.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_leds.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_logic_analyzer.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_sdir.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_usbhost.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_glitchkit_simple.c
	${CMAKE_CURRENT_SOURCE_DIR}/legacy_apis/usb_api_DS18B20.c
	${CLASSES_SOURCES}
	${CLASSES_GLITCHKIT_SOURCES}
	${SOURCE_COMMON_M4}
	)

add_dependencies(obj_greatfet_usb greatfet_usb_m0.bin) # m0_bin.s `incbin`s greatfet_usb_m0.bin
target_include_directories(obj_greatfet_usb PRIVATE ${PATH_LIBOPENCM3}/include ${PATH_GREATFET_FIRMWARE_COMMON} ${PATH_LIBGREAT}/firmware/include ${PATH_LIBGREAT}/firmware/include/platform/${LIBGREAT_PLATFORM})
target_compile_options(obj_greatfet_usb PRIVATE ${FLAGS_COMPILE_COMMON} ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4})
target_compile_definitions(obj_greatfet_usb PRIVATE ${DEFINES_COMMON} LPC43XX_M4)

# SPI flash version
add_executable(greatfet_usb.elf $<TARGET_OBJECTS:obj_greatfet_usb> $<TARGET_OBJECTS:libgreat>)
target_link_options(greatfet_usb.elf PRIVATE ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4} ${FLAGS_LINK_COMMON} ${FLAGS_LINK_M4_COMMON} ${FLAGS_LINK_M4_SPI})
target_link_directories(greatfet_usb.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/${LIBGREAT_PLATFORM}/linker)
target_link_libraries(greatfet_usb.elf c nosys opencm3_lpc43xx m)

# SPI ELF -> bin
add_custom_target(greatfet_usb.bin ALL DEPENDS greatfet_usb.elf COMMAND ${CMAKE_OBJCOPY} -Obinary greatfet_usb.elf greatfet_usb.bin)


# DFU version - using a different LD script to run directly from RAM
add_executable(greatfet_usb_dfu.elf $<TARGET_OBJECTS:obj_greatfet_usb> $<TARGET_OBJECTS:libgreat>)
#target_link_options(greatfet_usb_dfu.elf PRIVATE ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4} ${FLAGS_LINK_COMMON} ${FLAGS_LINK_M4_COMMON} ${FLAGS_LINK_M4_DFU})
target_link_options(greatfet_usb_dfu.elf PRIVATE ${FLAGS_LINK_COMMON} ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4} ${FLAGS_LINK_M4_COMMON} ${FLAGS_LINK_M4_DFU})
target_link_directories(greatfet_usb_dfu.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/${LIBGREAT_PLATFORM}/linker)
target_link_libraries(greatfet_usb_dfu.elf c nosys opencm3_lpc43xx m)

# DFU ELF -> bin
add_custom_target(greatfet_usb_dfu.bin ALL DEPENDS greatfet_usb_dfu.elf COMMAND ${CMAKE_OBJCOPY} -Obinary greatfet_usb_dfu.elf greatfet_usb_dfu.bin)

# bin -> DFU

add_custom_target(greatfet_usb.dfu ${DFU_ALL} DEPENDS greatfet_usb_dfu.bin
	COMMAND rm -f _tmp.dfu _header.bin
	COMMAND cp greatfet_usb_dfu.bin _tmp.dfu
	COMMAND ${DFU_COMMAND}
	COMMAND python ${PATH_GREATFET_FIRMWARE}/dfu.py greatfet_usb
	COMMAND cat _header.bin _tmp.dfu >greatfet_usb.dfu
	)


# Program/flash targets
add_custom_target(greatfet_usb-flash DEPENDS greatfet_usb.bin COMMAND greatfet_firmware -v -Rw greatfet_usb.bin)
add_custom_target(greatfet_usb-program DEPENDS greatfet_usb.dfu COMMAND dfu-util --device 1fc9:000c --alt 0 --download greatfet_usb.dfu)
