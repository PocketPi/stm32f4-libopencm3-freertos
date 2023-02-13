cmake_minimum_required(VERSION 3.15)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_TOOLCHAIN_FILE cmake/arm-gcc-toolchain.cmake)

project(firmware)

include(cmake/freertos.cmake)
include(cmake/libopencm3.cmake)

add_executable(firmware src/main.c)

target_link_libraries(firmware stm32f4)