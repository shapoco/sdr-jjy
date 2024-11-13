cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(jjymon C CXX ASM)

# Initialize the SDK
pico_sdk_init()

add_executable(jjymon)

target_compile_options(jjymon PUBLIC -O2 -Wall)
target_compile_features(jjymon PUBLIC c_std_11)
target_compile_features(jjymon PUBLIC cxx_std_17)

set(SRC_DIR src)
set(LIB_DIR ../../lib)

file(GLOB CPP_FILES
    ${LIB_DIR}/shapoco/*.cpp
    ${LIB_DIR}/shapoco/graphics/*.cpp
    ${LIB_DIR}/shapoco/pico/*.cpp
    ${LIB_DIR}/shapoco/jjy/*.cpp
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/fonts/*.cpp
)

target_sources(
    jjymon PRIVATE 
    ${CPP_FILES}
)

target_include_directories(
    jjymon PRIVATE
    ${SRC_DIR}
    ${LIB_DIR}
)

target_link_libraries(
    jjymon PRIVATE
    pico_stdlib
    pico_multicore
    hardware_dma
    hardware_pwm
    hardware_adc
    hardware_spi
    hardware_i2c
)

pico_enable_stdio_usb(jjymon 1)
pico_enable_stdio_uart(jjymon 0)

pico_add_extra_outputs(jjymon)
