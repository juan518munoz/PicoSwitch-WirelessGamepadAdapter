add_library(bluepad BluepadController.cpp)
target_link_libraries(bluepad pico_stdlib controller)
target_include_directories(bluepad PUBLIC ${PROJECT_SOURCE_DIR}/include)