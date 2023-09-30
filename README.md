# PicoSwitch-WirelessGamepadAdapter
Use any bluetooth gamepad on your Nintendo Switch with a Raspberry Pi Pico W.

This project is possible thanks to [Bluepad32](https://github.com/ricardoquesada/bluepad32).

https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/assets/62400508/e1148028-f1f3-4d5b-980a-72534b42acf7

## Installing
1. Download latest `.uf2` file from [releases](https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/releases).
2. Plug Pico on PC while holding the bootsel button.
3. A folder will appear, drag and drop the `.uf2` file inside it.
   ![image](https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/assets/62400508/9185e9d4-0b41-44cb-83b8-f706c67d144c)

## Known issues
- Only gamepad tested as of now is Series X, no issues shown so far.

## Building
1. Run `build.sh` to build the project, better instructions will be added later.

## Development roadmap
- [x] Bluetooth connection.
- [x] Basic button mapping.
- [x] Complete button mapping.
- [ ] Support multiple gamepads at once ([see example](https://github.com/raspberrypi/tinyusb/tree/e0aa405d19e35dbf58cf502b8106455c1a3c2a5c/examples/device/hid_composite)).
- [ ] Update Bluepad32 to latest version.
- [ ] Support other platforms.

## Acknowledgements
- [ricardoquesada](https://github.com/ricardoquesada) - maker of [Bluepad32](https://github.com/ricardoquesada/bluepad32)
- [lohengrin](https://github.com/lohengrin/) - port of [Bluepad32](https://github.com/lohengrin/Bluepad32_PicoW) to the Raspberry Pi Pico W.
- [splork](https://github.com/aveao/splork) and [retro-pico-switch](https://github.com/DavidPagels/retro-pico-switch) - for the hid descriptors and TinyUsb usage.

