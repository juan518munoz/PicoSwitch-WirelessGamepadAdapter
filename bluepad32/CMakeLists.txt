file(GLOB_RECURSE INCLUDES "include/*.h")
set(headers ${includes})

file(GLOB_RECURSE SOURCES "*.c")
set(srcs ${SOURCES})

add_library(bluepad32 ${srcs})

target_include_directories(bluepad32 PUBLIC ./include)

target_link_libraries(bluepad32
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_classic
    pico_btstack_cyw43
)