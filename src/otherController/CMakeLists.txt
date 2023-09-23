add_library(controller INTERFACE)
target_link_libraries(controller INTERFACE pico_stdlib hardware_pio)
add_subdirectory(bluepad)
pico_generate_pio_header(controller ${CMAKE_CURRENT_LIST_DIR}/Controller.pio)