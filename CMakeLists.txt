cmake_minimum_required(VERSION 3.13)

# PICO_BOARD should be set to 'pico_w', or not set at all.
if(DEFINED PICO_BOARD AND NOT ${PICO_BOARD} STREQUAL "pico_w")
    message(FATAL_ERROR "PICO_BOARD must be set to 'pico_w' or not set at all")
else()
    set(PICO_BOARD "pico_w")
endif()

# set PICO_SDK to be fetched from github if PICO_SDK_PATH is not set
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_FETCH_FROM_GIT on)
endif()

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

set(BLUEPAD32_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/bluepad32)

# To use BTstack from Pico SDK do
# set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)
# To use it from Bluepad32 (up-to-date, with custom patches for controllers):
set(BTSTACK_ROOT ${BLUEPAD32_ROOT}/external/btstack)

project(PicoSwitchWGA C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(PicoSwitchWGA ${SOURCES})

target_include_directories(PicoSwitchWGA PRIVATE 
    src 
    ${BLUEPAD32_ROOT}/src/components/bluepad32/include)
include_directories(include)

target_link_libraries(PicoSwitchWGA 
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_classic
    pico_btstack_ble
    pico_btstack_cyw43
    bluepad32
    tinyusb_device
    tinyusb_board
)

add_subdirectory(${BLUEPAD32_ROOT}/src/components/bluepad32 libbluepad32)

pico_enable_stdio_usb(PicoSwitchWGA 0)
pico_enable_stdio_uart(PicoSwitchWGA 0)
pico_add_extra_outputs(PicoSwitchWGA)
