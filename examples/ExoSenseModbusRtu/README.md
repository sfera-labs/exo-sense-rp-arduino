# Exo Sense RP Modbus RTU

This sketch turns [Exo Sense RP](https://www.sferalabs.cc/product/exo-sense-rp/) into a standard Modbus RTU slave device with access to all its functionalities over RS-485.

It requires the [Modbus RTU Slave library](https://github.com/sfera-labs/arduino-modbus-rtu-slave) to be installed.

## Configuration

Before uploading the sketch, you can modify the default configuration setting the `CFG_MB_*` defines in [config.h](config.h), where you find the relative documentation too.

All configuration parameters can then be modified via Modbus using the configuration registers listed below.

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

Upon error or no data available, registers values are set to `0xFFFF` (for unsigned short data type registers) or `0x8000` (for signed short).

|Address|R/W|Functions|Size (bits)|Data type|Unit|Description|
|------:|:-:|---------|-----------|---------|----|-----------|
|10|R|4|16|unsigned short|-|Sketc version. MSB = major, LSB = minor (e.g. 0x010A = ver 1.10)|
|101|R|2|1|-|-|Digital input DI1 state|
|102|R|2|1|-|-|Digital input DI2 state|
|201|R/W|1,5|1|-|-|Digital output DO1 state|
|211|W|6,16|16|unsigned short|s/10|Close DO1 for the specified time|
|301|R|4|16|signed short|&deg;C/10|Temperature|
|302|R|4|16|unsigned short|&permil;|Relative humidity|
|304|R|4|16|unsigned short|-|VOC sensor raw value|
|305|R|4|16|unsigned short|-|VOC index. Represents an air quality value on a scale from 0 to 500 where a lower value represents cleaner air and a value of 100 represents the typical air composition over the past 24h|
|307|R|4|16|unsigned short|lx/10|Light intensity|
|309|R|4|16|unsigned short|-|PIR sensor input counter. Incremented on each rising edge (i.e. when movement is detected). Rolls back to 0 after 65535|
|311|R|4|16|signed short|dB/10|[Sound evaluation *Leq*](https://github.com/sfera-labs/knowledge-base/blob/main/soundeval/equivalent-continuous-sound-level-leq.md) minimum value since last read (based on selected weightings configuration, see below)|
|312|R|4|16|signed short|dB/10|[Sound evaluation *Leq*](https://github.com/sfera-labs/knowledge-base/blob/main/soundeval/equivalent-continuous-sound-level-leq.md) maximum value since last read (based on selected weightings configuration, see below)|
|313|R|4|16|signed short|dB/10|[Sound evaluation *Leq*](https://github.com/sfera-labs/knowledge-base/blob/main/soundeval/equivalent-continuous-sound-level-leq.md) average value since last read (based on selected weightings configuration, see below)|
|401|W|6,16|16|unsigned short|s/10|Buzzer beep with specified duration|
|501|R/W|1,5|1|-|-|LED state|
|1000|W|6,16|16|unsigned short|-|Write `0xABCD` (or modified value set in `CFG_COMMIT_VAL` in [config.h](config.h)) to commit the new configuration written in the registers below. This register can only be written individually, i.e. using function 6, or function 16 with a single data value. After positive response the unit is restarted and the new configuration is applied|
|1001|R/W|3,6,16|16|unsigned short|-|New configuration - Modbus unit address|
|1002|R/W|3,6,16|16|unsigned short|-|New configuration - Modbus baud rate:<br/>`1` = 1200<br/>`2` = 2400<br/>`3` = 4800<br/>`4` = 9600<br/>`5` = 19200<br/>`6` = 38400<br/>`7` = 57600<br/>`8` = 115200|
|1003|R/W|3,6,16|16|unsigned short|-|New configuration - Modbus parity and stop bits:<br/>`1` = parity even, 1 stop bit<br/>`2` = parity odd, 1 stop bit<br/>`3` = parity none, 2 stop bits|
|1004|R/W|3,6,16|16|unsigned short|-|New configuration - Sound evaluation time weighting:<br/>`1` = SLOW<br/>`2` = FAST<br/>`3` = IMPULSE|
|1005|R/W|3,6,16|16|unsigned short|-|New configuration - Sound evaluation frequency weighting:<br/>`1` = A<br/>`2` = C<br/>`3` = Z|
|1006|R/W|3,6,16|16|signed short|&deg;C/10|New configuration - Temperature offset used to compensate temperature and relative humidity measurements|

