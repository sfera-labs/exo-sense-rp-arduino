# Exo Sense RP - Arduino IDE libraries and examples

Arduino IDE libraries and examples for [Exo Sense RP](https://www.sferalabs.cc/product/exo-sense-rp/) - the multi-sensor module based on the Raspberry Pi RP2040 (Pico) microcontroller.

## Arduino IDE setup

Install the Arduino IDE version 1.8.13 or newer

Install the Raspberry Pi RP2040 Arduino core available here:
https://github.com/earlephilhower/arduino-pico

## Library installation

- Open the Arduino IDE
- Download this repo: `git clone --depth 1 --recursive https://github.com/sfera-labs/exo-sense-rp-arduino`
- Go to the menu *Sketch* > *Include Library* > *Add .ZIP Library...*
- Select the downloaded folder

After installation you will see the example sketches under the menu *File* > *Examples* > *Exo Sense RP*.

## Uploading a sketch

Select "Generic RP2040" from the menu *Tools* > *Board* > *Raspberry Pi RP2040 Boards*.
You will see additional menu entries under *Tools*, set Flash size to 16MB (with or without FS) and leave the other entries unchanged.

The first time you upload a sketch, you will need to hold the BOOTSEL button down while plugging the USB cable connected to your computer, then release the BOOTSEL button.
Hit the Arduino IDE's upload button, the sketch will be transferred and start to run.

After the first upload, the board will appear under the standard Serial ports list and will automatically reset and switch to bootloader mode when hitting the IDE's upload button as with any other Arduino boards.

## Library usage

You can inport the library in your sketch with:

    #include <ExoSense.h>

Refer to the [examples](./examples) for usage details.
