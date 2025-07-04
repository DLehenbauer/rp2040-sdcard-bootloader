# Pico SD Card Bootloader

A bootloader for the Raspberry Pi Pico (RP2040) that allows updating firmware from a SD card.

## Overview

This bootloader enables RP2040 firmware updates using a SD card instead of USB.
Useful for designs where the USB port is inaccessible, otherwise occupied, or it is inconvenient to use a laptop to update firmware.

### Key Features

- Uses standard .uf2 files for firmware updates
- No custom linker script required
- Sanity checks firmware before flashing

## Quick Start

Note: the prebuilt binaries only work with the original Raspberry Pi Pico board.  If using a different board,
you may need to modify [config.cmake](config.cmake) and build bootloader.uf2 locally.

### 1. Connect a SD adapter

Connect a 3.3V-compatible SD adapter to your Raspberry Pi Pico:

| Pico Pin | GPIO      | Adapter Pin | Description             |
|----------|-----------|-------------|-------------------------|
| 21       | 16 (RX)   | DO          | Data out (from card)    |
| 22       | 17        | CS/SS       | Chip select             |
| 24       | 18 (SCK)  | SCK         | Serial clock            |
| 25       | 19 (TX)   | DI          | Data in (to card)       |
| 23       | GND       | GND/VSS     | Ground                  |
| 36       | 3V3 (Out) | 3V3/VCC     | Power                   |
| 29       | 22 (In)   | CD/DAT3     | Card detect (optional)  |

### 2. Install Bootloader (One-Time Setup)

1. Connect the Pico via USB while holding the BOOTSEL button
2. Copy [bootloader.uf2](dist/bootloader.uf2) to the mounted USB drive
3. The Pico will restart and flash "N" in Morse code (long-short pattern) indicating no firmware

### 3. Update firmware via SD card

1. Copy the example [firmware.uf2](dist/firmware.uf2) to a FAT32-formatted SD card
2. Insert the card into the adapter connected to your Pico
3. The LED will blink rapidly during flashing, then the device will restart
4. When successful, the LED will "breathe" as it fades in and out

## How It Works

The normal [RP2040 boot process](https://vanhunteradams.com/Pico/Bootloader/Boot_sequence.html) is as follows:

1. The ROM bootloader runs loads the stage 2 bootloader from the first block of flash.
2. The stage 2 bootloader configures the flash for XIP (execute in place) and jumps to the main program.

This bootloader adds a third stage to the RP2040 boot sequence that checks for firmware updates on an SD card and flashes them when detected:

1. The ROM bootloader runs the stage 2 bootloader as normal
2. A modified stage 2 bootloader jumps to a new stage 3 bootloader that resides in the last 64kB of flash
3. The stage 3 bootloader checks for an inserted SD card with a 'firmware.uf2' file
4. If found, it validates the UF2 file and writes it to flash.
5. The stage 3 bootloader jumps to the normal program area.

During firmware updates, the bootloader preserves itself by restoring its modified stage 2 bootloader and protecting the last 64kB of flash where it resides.

## Important Notes

* **USB/SWD Flashing Overwrites Bootloader**: The bootloader only preserves itself during SD card updates. If you flash the Pico using USB or an SWD debugger (like Picoprobe or Debug Probe), it will overwrite the custom bootloader. To restore SD card update functionality, reinstall [bootloader.uf2](dist/bootloader.uf2).

* **Read-Only Firmware File**: If the 'firmware.uf2' file is read-only, the bootloader cannot delete it after flashing. This is useful for updating multiple devices with the same card but may slow startup times if the card remains inserted, as the bootloader checks the UF2 file against installed firmware on each boot.

## Customizing

Modify [config.cmake](config.cmake) to configure the following:

* Board (defaults to 'pico')
* Firmware filename (defaults to 'firmware.uf2')
* SD card SPI instance, pins, and optional card detection
* Diagnostics options:
  * Enable/disable LED diagnostic codes and select LED pin
  * Enable/disable serial UART diagnostics and select TX/RX pins and baud rate

## Related Projects

* [Hachi (八)](https://github.com/muzkr/hachi)
* [Pico SD Bootloader](https://github.com/julienfdev/pico-sd-bootloader)
* [Raspberry Pi SD Card Bootloader](https://github.com/oyama/pico-sdcard-boot)
* [Picocalc_SD_Boot](https://github.com/adwuard/Picocalc_SD_Boot)

## License

This project is licensed under the [0BSD license](https://opensource.org/licenses/0BSD), except where noted in source files or [NOTICE.md](NOTICE.md).
