cmake_minimum_required(VERSION 3.15)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(firmware)

include(FetchContent)

FetchContent_Declare( freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG        main
)

add_library(freertos_config INTERFACE)

target_include_directories(freertos_config SYSTEM
INTERFACE
./
)

set( FREERTOS_PORT "GCC_POSIX" CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS "-Wall -Wundef")


FetchContent_MakeAvailable(freertos_kernel)

add_executable(firmware src/main.c)