# Exo Sense RP - Arduino IDE libraries and examples

Arduino IDE libraries and examples for [Exo Sense RP](https://www.sferalabs.cc/product/exo-sense-rp/) - the multi-sensor module based on the Raspberry Pi RP2040 (Pico) microcontroller.

## Arduino IDE setup

Install the Arduino IDE version 1.8.13 or newer

Install the Raspberry Pi RP2040 Arduino core available here:
https://github.com/earlephilhower/arduino-pico

## Library installation

- Download this repo with: `git clone --depth 1 --recursive https://github.com/sfera-labs/exo-sense-rp-arduino`
- Open the Arduino IDE
- Go to the menu *Sketch* > *Include Library* > *Add .ZIP Library...*
- Select the downloaded folder

After installation you will see the example sketches under the menu *File* > *Examples* > *Exo Sense RP*.

## Uploading a sketch

Go to the menu *Tools* > *Board* > *Raspberry Pi RP2040 Boards* and select "Generic RP2040" (end of the list).    
You will see additional menu entries under the *Tools* menu, set:
- *Flash size* to 16MB (with or without FS);
- *Boot Stage 2* to "W25Q128JV QSPI /4";
- leave the other entries unchanged.

The **first** time you upload a sketch:
- Remove power to Exo Sense RP
- Connect the USB cable to Exo Sense RP
- Connect a wire jumper to the BOOTSEL CN3 header
- Turn on power supply to the Exo Sense RP
- Remove the BOOTSEL jumper

Select "UF2 Board" as upload Port then hit the Arduino IDE's upload button, the sketch will be transferred and start to run.

After the first upload, the board will appear under the standard Serial ports list and will automatically reset and switch to bootloader mode when hitting the IDE's upload button as with any other Arduino boards.

## Library usage

You can inport the library in your sketch with:

    #include <ExoSense.h>

Refer to the [examples](./examples) for usage details.
