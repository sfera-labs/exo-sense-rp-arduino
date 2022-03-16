# Exo Sense RP Modbus RTU

This sketch turns [Exo Sense RP](https://www.sferalabs.cc/product/exo-sense-rp/) into a standard Modbus RTU slave device with access to all its functionalities over RS-485.

It requires the [Modbus RTU Slave library](https://github.com/sfera-labs/arduino-modbus-rtu-slave) to be installed.

## Configuration

Before uploading the sketch, edit the configuration defines contained in the [config.h](config.h) file, together with their documentation.

## Modbus registers

Refer to the following table for the list of available registers and corresponding supported Modbus functions.

For the "Functions" column:    
1 = Read coils    
2 = Read discrete inputs    
3 = Read holding registers    
4 = Read input registers    
5 = Write single coil    
6 = Write single register    
15 = Write multiple coils    
16 = Write multiple registers    

|Address|R/W|Functions|Size (bits)|Data type|Unit|Description|
|------:|:-:|---------|----|---------|----|-----------|
|101|R|2|1|-|-|Digital input DI1 state|
|102|R|2|1|-|-|Digital input DI2 state|
|201|R/W|1,5|1|-|-|Digital output DO1 state|
|211|W|6|16|unsigned short|s/10|Close DO1 for the specified time|
|301|R|4|16|signed short|&deg;C/10|Temperature|
|302|R|4|16|unsigned short|&permil;|Relative humidity|
|304|R|4|16|unsigned short|-|VOC sensor raw value|
|305|R|4|16|unsigned short|-|VOC index. Represents an air quality value on a scale from 0 to 500 where a lower value represents cleaner air and a value of 100 represents the typical air composition over the past 24h|
|307|R|4|16|unsigned short|lx/10|Light intensity|
|309|R|4|16|unsigned short|-|PIR sensor input counter. Incremented on each rising edge (i.e. when movement is detected). Rolls back to 0 after 65535|
|311|R|4|16|signed short|dB/10|LEQ period evaluation result minimum since last read|
|312|R|4|16|signed short|dB/10|LEQ period evaluation result maximum since last read|
|313|R|4|16|signed short|dB/10|LEQ period evaluation result average since last read|
|315|R|4|16|signed short|dB/10|LEQ interval evaluation result minimum since last read|
|316|R|4|16|signed short|dB/10|LEQ interval evaluation result maximum since last read|
|317|R|4|16|signed short|dB/10|LEQ interval evaluation result average since last read|
|401|W|6|16|unsigned short|s/10|Buzzer beep with specified duration|
|501|R/W|1,5|1|-|-|LED state|

Upon error or no data available, registers values are set to `0xFFFF` (for unsigned short data type registers) or `0x8000` (signed short).
