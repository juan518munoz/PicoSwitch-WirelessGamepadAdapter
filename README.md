# PicoSwitch-WirelessGamepadAdapter
Use any bluetooth gamepad on your Nintendo Switch with a Raspberry Pi Pico W.

This project is possible thanks to [Bluepad32](https://github.com/ricardoquesada/bluepad32) and [TinyUSB](https://github.com/hathach/tinyusb).

https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/assets/62400508/e1148028-f1f3-4d5b-980a-72534b42acf7

Multiple gamepads support (4 max)

https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/assets/62400508/247b4c35-10f9-402a-9c81-fd966c905dd6

## Installing
1. Download latest `.uf2` file from [releases](https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/releases).
2. Plug Pico on PC while holding the bootsel button.
3. A folder will appear, drag and drop the `.uf2` file inside it.
   ![image](https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter/assets/62400508/9185e9d4-0b41-44cb-83b8-f706c67d144c)

## Known issues
- Only gamepad tested as of now is Series X, no issues shown so far.

## Building
1. Clone the repository with `git clone https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter`
2. Enter the repo directory with `cd PicoSwitch-WirelessGamepadAdapter`
3. Setup the Pico SDK download with `export PICO_SDK_FETCH_FROM_GIT=on`
4. Run `./build.sh` (This may take a while)
5. The `.uf2` file should be inside the newly created `build` directory at `./build/PicoSwitchWGA.uf2`. Follow the installation steps above

## Development roadmap
- [x] Bluetooth connection.
- [x] Basic button mapping.
- [x] Complete button mapping.
- [x] Support multiple gamepads at once (needs better testing).
- [x] Update Bluepad32 to latest version (only partial update, parsers not updated).
- [ ] Support other platforms.

## Acknowledgements
- [ricardoquesada](https://github.com/ricardoquesada) - maker of [Bluepad32](https://github.com/ricardoquesada/bluepad32)
- [lohengrin](https://github.com/lohengrin/) - port of [Bluepad32](https://github.com/lohengrin/Bluepad32_PicoW) to the Raspberry Pi Pico W.
- [hathach](https://github.com/hathach) creator of [TinyUSB](https://github.com/hathach/tinyusb)
- [splork](https://github.com/aveao/splork) and [retro-pico-switch](https://github.com/DavidPagels/retro-pico-switch) - for the hid descriptors and TinyUsb usage.

