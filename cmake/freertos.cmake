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

set( FREERTOS_PORT "GCC_ARM_CM0" CACHE STRING "" FORCE)


set(CMAKE_C_FLAGS                 "-mcpu=cortex-m0plus -mthumb -msoft-float --static -nostartfiles -fno-common" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS               "-mcpu=cortex-m0plus -mthumb -msoft-float --static -nostartfiles -fno-common" CACHE INTERNAL "")

FetchContent_MakeAvailable(freertos_kernel)