#
# This file is part of GreatFET
#

cmake_minimum_required(VERSION 3.1.3)

project(blinky C CXX ASM)

# -------M0 Stuff-------

set(SOURCE_M0 ${PATH_GREATFET_FIRMWARE_COMMON}/m0_sleep.c)

configure_file(${PATH_GREATFET_FIRMWARE}/cmake/m0_bin.s.cmake m0_bin.s)

add_executable(blinky_m0.elf ${SOURCE_M0})

target_compile_options(blinky_m0.elf PRIVATE -std=gnu99 -Os -g3 -Wall -Wextra -fno-common -MD -fno-builtin-printf -Wno-missing-field-initializers)

# CPU flags
target_compile_options(blinky_m0.elf PRIVATE -mthumb -mcpu=cortex-m0 -mfloat-abi=soft)

target_compile_definitions(blinky_m0.elf PRIVATE GREATFET_ONE LPC43XX LPC4330 LPC43XX_M0)

target_link_options(blinky_m0.elf PRIVATE -nostartfiles -Wl,--gc-sections -mthumb -mcpu=cortex-m0 -mfloat-abi=soft -TLPC43xx_M0_memory.ld -Tlibopencm3_lpc43xx_m0.ld -Xlinker -Map=m0.map)

target_link_directories(blinky_m0.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/lpc43xx/linker)

target_link_libraries(blinky_m0.elf PRIVATE c nosys opencm3_lpc43xx_m0)

# ELF -> bin
add_custom_target(blinky_m0.bin ALL DEPENDS blinky_m0.elf COMMAND ${CMAKE_OBJCOPY} -Obinary blinky_m0.elf blinky_m0.bin)


# -------M4 Stuff-------

set(SOURCE_COMMON_M4
	${PATH_GREATFET_FIRMWARE_COMMON}/greatfet_core.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spiflash_target.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spiflash.c
	${PATH_GREATFET_FIRMWARE_COMMON}/spi_ssp.c
	${PATH_GREATFET_FIRMWARE_COMMON}/i2c_bus.c
	${PATH_GREATFET_FIRMWARE_COMMON}/i2c_lpc.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_lpc.c
	${PATH_GREATFET_FIRMWARE_COMMON}/gpio_int.c
	${PATH_GREATFET_FIRMWARE_COMMON}/sgpio.c
	${PATH_GREATFET_FIRMWARE_COMMON}/debug.c
	${PATH_GREATFET_FIRMWARE_COMMON}/printf.c
	${PATH_GREATFET_FIRMWARE_COMMON}/time.c
	)

# SPI flash version
add_executable(blinky.elf ${CMAKE_CURRENT_SOURCE_DIR}/blinky.c ${SOURCE_COMMON_M4} m0_bin.s)
add_dependencies(blinky.elf blinky_m0.bin)

target_include_directories(blinky.elf PRIVATE ${PATH_LIBOPENCM3}/include ${PATH_GREATFET_FIRMWARE_COMMON} ${PATH_LIBGREAT}/firmware/include)

target_compile_options(blinky.elf PRIVATE -std=gnu99 -Os -g3 -Wall -Wextra -fno-common -MD -fno-builtin-printf -Wno-missing-field-initializers)

# CPU flags
target_compile_options(blinky.elf PRIVATE -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)

target_compile_definitions(blinky.elf PRIVATE GREATFET_ONE LPC43XX LPC4330 LPC43XX_M4)

target_link_options(blinky.elf PRIVATE -nostartfiles -Wl,--gc-sections -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -TLPC4330_M4_memory.ld -Tlibgreat_lpc43xx_rom_to_ram.ld -TLPC43xx_M4_M0_image_from_text.ld)

target_link_directories(blinky.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/lpc43xx/linker)

target_link_libraries(blinky.elf c nosys opencm3_lpc43xx m libgreat)


# ELF -> bin
add_custom_target(blinky.bin ALL DEPENDS blinky.elf COMMAND ${CMAKE_OBJCOPY} -Obinary blinky.elf blinky.bin)

# DFU - using a different LD script to run directly from RAM
add_executable(blinky_dfu.elf ${CMAKE_CURRENT_SOURCE_DIR}/blinky.c ${SOURCE_COMMON_M4} m0_bin.s)
add_dependencies(blinky_dfu.elf blinky_m0.bin)

target_include_directories(blinky_dfu.elf PRIVATE ${PATH_LIBOPENCM3}/include ${PATH_GREATFET_FIRMWARE_COMMON} ${PATH_LIBGREAT}/firmware/include ${PATH_LIBGREAT}/firmware/platform/lpc43xx/linker)

target_compile_options(blinky_dfu.elf PRIVATE -std=gnu99 -Os -g3 -Wall -Wextra -fno-common -MD -fno-builtin-printf -Wno-missing-field-initializers)

# CPU flags
target_compile_options(blinky_dfu.elf PRIVATE -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)

target_compile_definitions(blinky_dfu.elf PRIVATE GREATFET_ONE LPC4XX LPC4330 LPC43XX_M4)

target_link_options(blinky_dfu.elf PRIVATE -nostartfiles -Wl,--gc-sections -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -TLPC4330_M4_memory.ld -Tlibgreat_lpc43xx.ld -TLPC43xx_M4_M0_image_from_text.ld -Xlinker -Map=m4.map)

target_link_directories(blinky_dfu.elf PRIVATE ${PATH_LIBOPENCM3}/lib ${PATH_LIBOPENCM3}/lib/lpc43xx ${PATH_LIBGREAT}/firmware/platform/lpc43xx/linker)

target_link_libraries(blinky_dfu.elf c nosys opencm3_lpc43xx m libgreat)

add_custom_target(blinky_dfu.bin DEPENDS blinky_dfu.elf COMMAND ${CMAKE_OBJCOPY} -Obinary blinky_dfu.elf blinky_dfu.bin)

add_custom_target(blinky.dfu ${DFU_ALL} DEPENDS blinky_dfu.bin
	COMMAND rm -f _tmp.dfu _header.bin
	COMMAND cp blinky_dfu.bin _tmp.dfu
	COMMAND ${DFU_COMMAND}
	COMMAND python ${PATH_GREATFET_FIRMWARE_COMMON}/../dfu.py blinky
	COMMAND cat _header.bin _tmp.dfu >blinky.dfu
	)


# Program/flash targets
add_custom_target(blinky-flash DEPENDS blinky.bin COMMAND greatfet_firmware -v -Rw blinky.bin)

add_custom_target(blinky-program DEPENDS blinky.dfu COMMAND dfu-util --device 1fc9:000c --alt 0 --download blinky.dfu)
