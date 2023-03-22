set(includes
./sdkconfig.h
./include/uni_hid_device.h
./include/uni_circular_buffer.h
./include/uni_console.h
./include/uni_hci_cmd.h
./include/uni_hid_parser_ds4.h
./include/uni_platform_unijoysticle.h
./include/uni_property_mem.h
./include/uni_platform_mightymiggy.h
./include/uni_platform_unijoysticle_amiga.h
./include/uni_platform_unijoysticle_c64.h
./include/uni_hid_parser_wii.h
./include/uni_bt_defines.h
./include/uni_bt_conn.h
./include/uni_controller.h
./include/uni_platform_arduino_bootstrap.h
./include/uni_bluetooth.h
./include/uni_ble.h
./include/uni_platform_pc_debug.h
./include/uni_hid_parser_8bitdo.h
./include/uni_hid_parser_ds5.h
./include/uni_property.h
./include/uni_log.h
./include/uni_hid_device_vendors.h
./include/uni_hid_parser_switch.h
./include/uni_hid_parser_icade.h
./include/uni_mouse_quadrature.h
./include/uni_debug.h
./include/uni_utils.h
./include/uni_platform_pico.h
./include/uni_balance_board.h
./include/uni_common.h
./include/uni_main.h
./include/uni_gamepad.h
./include/uni_hid_parser_generic.h
./include/uni_mouse.h
./include/uni_platform_nina.h
./include/uni_platform_arduino.h
./include/uni_hid_parser_mouse.h
./include/uni_gpio.h
./include/uni_bt_setup.h
./include/uni_config.h
./include/uni_bt_sdp.h
./include/uni_hid_parser_ds3.h
./include/uni_hid_parser_android.h
./include/uni_hid_parser_nimbus.h
./include/uni_hid_parser_ouya.h
./include/hid_usage.h
./include/uni_hid_parser_xboxone.h
./include/uni_platform.h
./include/uni_version.h
./include/uni_property_nvs.h
./include/uni_esp32.h
./include/uni_joystick.h
./include/uni_hid_parser.h
./include/uni_hid_parser_smarttvremote.h
./include/sdkconfig.h
)

set(srcs "uni_balance_board.c"
         "uni_ble.c"
         "uni_bluetooth.c"
         "uni_bt_conn.c"
         "uni_bt_sdp.c"
         "uni_bt_setup.c"
         "uni_circular_buffer.c"
#         "uni_console.c"
         "uni_controller.c"
#         "uni_esp32.c"
         "uni_gamepad.c"
#         "uni_gpio.c"
         "uni_hci_cmd.c"
         "uni_hid_device.c"
         "uni_hid_parser.c"
         "uni_hid_parser_8bitdo.c"
         "uni_hid_parser_android.c"
         "uni_hid_parser_ds3.c"
         "uni_hid_parser_ds4.c"
         "uni_hid_parser_ds5.c"
         "uni_hid_parser_generic.c"
         "uni_hid_parser_icade.c"
         "uni_hid_parser_mouse.c"
         "uni_hid_parser_nimbus.c"
         "uni_hid_parser_ouya.c"
         "uni_hid_parser_smarttvremote.c"
         "uni_hid_parser_switch.c"
         "uni_hid_parser_wii.c"
         "uni_hid_parser_xboxone.c"
         "uni_joystick.c"
         "uni_log.c"
         "uni_main.c"
         "uni_mouse.c"
#         "uni_mouse_quadrature.c"
         "uni_platform.c"
#         "uni_platform_arduino.c"
#         "uni_platform_arduino_bootstrap.cpp"
#         "uni_platform_mightymiggy.c"
#         "uni_platform_nina.c"
#         "uni_platform_pc_debug.c"
#         "uni_platform_unijoysticle.c"
#         "uni_platform_unijoysticle_amiga.c"
#         "uni_platform_unijoysticle_c64.c"
         "uni_property.c"
         "uni_property_mem.c"
#         "uni_property_nvs.c"
"uni_utils.c"
# PICO
         "uni_platform_pico.c"
         )

add_library(bluepad32
${srcs}
)

target_include_directories(bluepad32 PUBLIC ./include ../src/bt)
target_link_libraries(bluepad32
pico_stdlib
pico_cyw43_arch_none
pico_btstack_ble
pico_btstack_classic
pico_btstack_cyw43
)