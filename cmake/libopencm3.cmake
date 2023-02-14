include(FetchContent)

FetchContent_Declare( libopencm3
  GIT_REPOSITORY https://github.com/libopencm3/libopencm3.git
  GIT_TAG        master
)

FetchContent_MakeAvailable(libopencm3)
FetchContent_GetProperties(libopencm3)

add_custom_target(libopencm3_make_target make -j WORKING_DIRECTORY ${libopencm3_SOURCE_DIR})

add_library(libopencm3 STATIC IMPORTED)
target_link_libraries(libopencm3 INTERFACE)

set_property(TARGET libopencm3 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${libopencm3_SOURCE_DIR}/include)
set_property(TARGET libopencm3 PROPERTY IMPORTED_LOCATION ${libopencm3_SOURCE_DIR}/lib/libopencm3_stm32g0.a)
add_dependencies(libopencm3 libopencm3_make_target)
target_link_directories(libopencm3 INTERFACE ${libopencm3_SOURCE_DIR}/lib)

target_compile_definitions(libopencm3 INTERFACE -DSTM32G0)

set(CMAKE_C_FLAGS                 "-mcpu=cortex-m0plus -mthumb -msoft-float --static -nostartfiles -fno-common" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS               "-mcpu=cortex-m0plus -mthumb -msoft-float --static -nostartfiles -fno-common" CACHE INTERNAL "")
