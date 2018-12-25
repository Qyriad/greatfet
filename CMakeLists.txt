cmake_minimum_required(VERSION 3.1.3)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/firmware/cmake/toolchain-arm-cortex-m.cmake)

project(greatfet)

include(${CMAKE_CURRENT_LIST_DIR}/firmware/cmake/dfu-util.cmake)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libgreat)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/firmware)
