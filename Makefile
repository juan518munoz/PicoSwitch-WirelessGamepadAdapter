.PHONY: all build clean flash
.ONESHELL:

COLOUR_GREEN=\033[0;32m
COLOUR_RED=\033[0;31m
END_COLOUR=\033[0m

all: build flash

build:
	@rm -rf build
	@mkdir build && cd build 
	@cmake .. -DPICO_BOARD=pico_w
	@make -j

format: .clang-files .clang-format
	@xargs -r clang-format -i <$<

update:
	@git submodule update --init --recursive
	@cd ${PICO_SDK_PATH} && git submodule update --init

flash:
	@echo "$(COLOUR_GREEN)Flashing uf2 file to Pico W$(END_COLOUR)"
	@cp build/PicoSwitchWGA.uf2 /media/${USER}/RPI-RP2

flash_nuke:
	@echo "$(COLOUR_RED)Cleaning Pico W flash storage$(END_COLOUR)"
	@cp flash_nuke.uf2 /media/${USER}/RPI-RP2

clean:
	@rm -rf build

debug:
	sudo minicom -D /dev/ttyACM0	
