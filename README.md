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
1. Install Make, CMake (at least version 3.13), and GCC cross compiler
   ```bash
   sudo apt-get install make cmake gdb-arm-none-eabi gcc-arm-none-eabi build-essential
   ```
2. (Optional) Install [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set `PICO_SDK_PATH` environment variable to the SDK path. Not using the SDK will download it automatically for each build.
3. Update submodules
   ```bash
   make update
   ```
4. Build
   ```bash
   make build
   ```
5. Flash!
   ```bash
   make flash
   ```
   This `make` command will only work on OSes where the mounted pico drive is located in `/media/${USER}/RPI-RP2`. If this is not the case, you can manually copy the `.uf2` file located inside the `build` directory to the pico drive.

#### Other `make` commands:
- `clean` - Clean build directory.
- `flash_nuke` - Flash the pico with `flash_nuke.uf2` which will erase the flash memory. This is useful when the pico is stuck in a boot loop.
- `all` - `build` and `flash`.
- `format` - Format the code using `clang-format`. This requires `clang-format` to be installed.
- `debug` - Start _minicom_ to debug the pico. This requires `minicom` to be installed and uart debugging.

## Development roadmap
- [x] Bluetooth connection.
- [x] Basic button mapping.
- [x] Complete button mapping.
- [x] Support multiple gamepads at once (needs better testing).
- [x] Update Bluepad32 to latest version.
- [x] Update Bluepad32 to latest version.
- [ ] Support other platforms.

## Acknowledgements
- [ricardoquesada](https://github.com/ricardoquesada) - maker of [Bluepad32](https://github.com/ricardoquesada/bluepad32)
- [hathach](https://github.com/hathach) creator of [TinyUSB](https://github.com/hathach/tinyusb)
- [splork](https://github.com/aveao/splork) and [retro-pico-switch](https://github.com/DavidPagels/retro-pico-switch) - for the hid descriptors and TinyUsb usage examples.

