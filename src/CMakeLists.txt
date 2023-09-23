# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    set(srcs "ArduinoBluepad32.cpp"
            "ArduinoConsole.cpp"
            "ArduinoController.cpp"
            "ArduinoGamepad.cpp")

    add_subdirectory(otherController)
    add_subdirectory(switchController)
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME} pico_stdlib tinyusb_device switchController bluepad)

    set(requires "bluepad32" "arduino")

    idf_component_register(SRCS ${srcs}
                        INCLUDE_DIRS "include"
                        REQUIRES ${requires})
endif()
