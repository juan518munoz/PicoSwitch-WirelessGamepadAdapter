# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    set(srcs "ArduinoBluepad32.cpp"
            "ArduinoConsole.cpp"
            "ArduinoController.cpp"
            "ArduinoGamepad.cpp")

    target_link_libraries(${PROJECT_NAME}
        pico_stdlib 
        tinyusb_device 
        tinyusb_board
        pico_btstack_classic
        pico_btstack_ble
        pico_btstack_cyw43
        pico_multicore
    )


    set(requires "bluepad32" "arduino")

    idf_component_register(SRCS ${srcs}
                        REQUIRES ${requires})

endif()
