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

set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)

set( FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS                 "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb --static -nostartfiles -fno-common -Wundef" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS               "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb --static -nostartfiles -fno-common -Wundef" CACHE INTERNAL "")


FetchContent_MakeAvailable(freertos_kernel)