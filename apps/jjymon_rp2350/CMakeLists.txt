cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(jjymon_rp2350 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(jjymon_rp2350)

set(SRC_DIR src)
set(LIB_DIR ../../lib)

file(GLOB LIB_CPP ${LIB_DIR}/shapoco/*.cpp)
file(GLOB LIB_JJY_CPP ${LIB_DIR}/shapoco/jjy/*.cpp)
file(GLOB LIB_PICO_CPP ${LIB_DIR}/shapoco/pico/*.cpp)
file(GLOB APP_CPP ${SRC_DIR}/*.cpp)
file(GLOB APP_FONTS_CPP ${SRC_DIR}/fonts/*.cpp)

target_sources(
    jjymon_rp2350 PRIVATE 
    ${LIB_CPP}
    ${LIB_JJY_CPP}
    ${LIB_PICO_CPP}
    ${APP_FONTS_CPP}
    ${APP_CPP}
)

target_include_directories(
    jjymon_rp2350 PRIVATE
    ${SRC_DIR}
    ${LIB_DIR}
)

target_link_libraries(
    jjymon_rp2350 PRIVATE
    pico_stdlib
    pico_multicore
    hardware_dma
    hardware_pwm
    hardware_adc
    hardware_spi
)

pico_enable_stdio_usb(jjymon_rp2350 1)
pico_enable_stdio_uart(jjymon_rp2350 0)

pico_add_extra_outputs(jjymon_rp2350)
