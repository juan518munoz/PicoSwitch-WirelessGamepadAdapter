cmake_minimum_required(VERSION 3.13)

#Defualt to pico_w
if(NOT DEFINED PICO_BOARD)
	message(FATAL_ERROR "This program is for Pico W board, please define PICO_BOARD to pico_w")
endif()

set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(Bluepad32_PicoW C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(Bluepad32_PicoW
    src/main.cpp
    src/bluepad_main.cpp
    src/usb_descriptors.c
    src/picoHandler/PicoBluepad32.cpp
    src/picoHandler/PicoConsole.cpp
    src/picoHandler/PicoController.cpp
    src/picoHandler/PicoGamepad.cpp
)

target_include_directories(Bluepad32_PicoW PRIVATE src include bluepad32/include)
target_link_libraries(Bluepad32_PicoW 
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_classic
    pico_btstack_ble
    pico_btstack_cyw43
    bluepad32
    tinyusb_device
    tinyusb_board
    pico_multicore
)

add_subdirectory(bluepad32)

pico_enable_stdio_usb(Bluepad32_PicoW 1)
pico_enable_stdio_uart(Bluepad32_PicoW 1)
pico_add_extra_outputs(Bluepad32_PicoW)
