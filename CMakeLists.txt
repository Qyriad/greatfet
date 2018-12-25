cmake_minimum_required(VERSION 3.1.3)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/firmware/cmake/toolchain-arm-cortex-m.cmake)

project(greatfet)

include(${CMAKE_CURRENT_LIST_DIR}/firmware/cmake/dfu-util.cmake)

set(PATH_GREATFET_FIRMWARE ${CMAKE_CURRENT_SOURCE_DIR}/firmware)
set(PATH_GREATFET_FIRMWARE_COMMON ${PATH_GREATFET_FIRMWARE}/common)
set(PATH_LIBGREAT ${CMAKE_CURRENT_SOURCE_DIR}/libgreat)
set(PATH_LIBOPENCM3 ${CMAKE_CURRENT_SOURCE_DIR}/firmware/libopencm3)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libgreat)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/firmware)
