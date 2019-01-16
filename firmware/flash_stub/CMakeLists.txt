#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

project(flash_stub C CXX ASM)

# -------M0 Stuff-------

set(SOURCE_M0 ${PATH_GREATFET_FIRMWARE_COMMON}/m0_sleep.c)

configure_file(${PATH_GREATFET_FIRMWARE}/cmake/m0_bin.s.cmake m0_bin.s)

add_executable(flash_stub_m0.elf ${SOURCE_M0})

target_compile_options(flash_stub_m0.elf PRIVATE ${FLAGS_COMPILE_COMMON} ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M0})
target_compile_definitions(flash_stub_m0.elf PRIVATE ${DEFINES_COMMON})
target_link_options(flash_stub_m0.elf PRIVATE ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M0} ${FLAGS_LINK_COMMON} ${FLAGS_LINK_M0})

target_link_directories(flash_stub_m0.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/lpc43xx/linker)

target_link_libraries(flash_stub_m0.elf PRIVATE c nosys opencm3_lpc43xx_m0)

# ELF -> bin
add_custom_target(flash_stub_m0.bin ALL DEPENDS flash_stub_m0.elf COMMAND ${CMAKE_OBJCOPY} -Obinary flash_stub_m0.elf flash_stub_m0.bin)


# -------M4 Stuff-------

# firmware/common sources used by flash_stub
set(SOURCE_COMMON_M4
	${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c
	${PATH_GREATFET_FIRMWARE_COMMON}/debug.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c
	${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c
	${PATH_GREATFET_FIRMWARE_COMMON}/greatfet_core.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spiflash_target.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spiflash.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spi_ssp.c
	${PATH_GREATFET_FIRMWARE_COMMON}/i2c_bus.c
	${PATH_GREATFET_FIRMWARE_COMMON}/i2c_lpc.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_lpc.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_int.c
	${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c
	${PATH_GREATFET_FIRMWARE_COMMON}/printf.c
	${PATH_GREATFET_FIRMWARE_COMMON}/time.c
	${PATH_GREATFET_FIRMWARE_COMMON}/glitchkit.c # required by libgreat
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_scu.c # required by glitchkit.c
	)

# firmware/greatfet_usb sources used by flash_stub
set(SOURCE_GREATFET_USB_M4
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/usb_bulk_buffer.c
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/usb_descriptor.c
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/usb_device.c
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/usb_endpoint.c
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/classes/core.c
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/classes/debug.c
	${PATH_GREATFET_FIRMWARE}/greatfet_usb/classes/firmware.c
	)

add_library(obj_flash_stub OBJECT
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
	${SOURCE_COMMON_M4}
	${SOURCE_GREATFET_USB_M4}
	)

add_dependencies(obj_flash_stub flash_stub_m0.bin) # m0_bin.s `incbin`s flash_stub_m0.bin
target_include_directories(obj_flash_stub PRIVATE ${PATH_LIBOPENCM3}/include ${PATH_GREATFET_FIRMWARE_COMMON} ${PATH_LIBGREAT}/firmware/include ${PATH_LIBGREAT}/firmware/include/platform/${LIBGREAT_PLATFORM})
target_compile_options(obj_flash_stub PRIVATE ${FLAGS_COMPILE_COMMON} ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4})
target_compile_definitions(obj_flash_stub PRIVATE ${DEFINES_COMMON} LPC43XX_M4)


# SPI flash version
add_executable(flash_stub.elf $<TARGET_OBJECTS:obj_flash_stub> $<TARGET_OBJECTS:libgreat>)
target_link_options(flash_stub.elf PRIVATE ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4} ${FLAGS_LINK_COMMON} ${FLAGS_LINK_M4_COMMON} ${FLAGS_LINK_M4_SPI})
target_link_directories(flash_stub.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/${LIBGREAT_PLATFORM}/linker)
target_link_libraries(flash_stub.elf c nosys opencm3_lpc43xx m)

# SPI ELF -> bin
add_custom_target(flash_stub.bin ALL DEPENDS flash_stub.elf COMMAND ${CMAKE_OBJCOPY} -Obinary flash_stub.elf flash_stub.bin)


# DFU version - using a different LD script to run directly from RAM
add_executable(flash_stub_dfu.elf $<TARGET_OBJECTS:obj_flash_stub> $<TARGET_OBJECTS:libgreat>)
target_link_options(flash_stub_dfu.elf PRIVATE ${FLAGS_LINK_COMMON} ${FLAGS_CPU_COMMON} ${FLAGS_CPU_M4} ${FLAGS_LINK_M4_COMMON} ${FLAGS_LINK_M4_DFU})
target_link_directories(flash_stub_dfu.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/${LIBGREAT_PLATFORM}/linker)
target_link_libraries(flash_stub_dfu.elf c nosys opencm3_lpc43xx m)

# DFU ELF -> bin
add_custom_target(flash_stub_dfu.bin ALL DEPENDS flash_stub_dfu.elf COMMAND ${CMAKE_OBJCOPY} -Obinary flash_stub_dfu.elf flash_stub_dfu.bin)

# bin -> DFU
add_custom_target(flash_stub.dfu ${DFU_ALL} DEPENDS flash_stub_dfu.bin
	COMMAND rm -f _tmp.dfu _header.bin
	COMMAND cp flash_stub_dfu.bin _tmp.dfu
	COMMAND python ${PATH_GREATFET_FIRMWARE}/dfu.py flash_stub
	COMMAND cat _header.bin _tmp.dfu >flash_stub.dfu)


# Program/flash targets
add_custom_target(flash_stub-flash DEPENDS flash_stub.bin COMMAND greatfet_firmware -v -Rw flash_stub.bin)
add_custom_target(flash_stub-program DEPENDS flash_stub.dfu COMMAND dfu-util --device 1fc9:000c --alt 0 --download flash_stub.dfu)
