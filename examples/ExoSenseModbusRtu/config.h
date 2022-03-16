// Modbus unit address: 1-255
#define CFG_MB_UNIT_ADDDR   10

// Modbus baudrate (bit/s)
#define CFG_MB_BAUDRATE     19200

// Modbus serial configuration (data bits, parity, and stop bits)
//  Format: SERIAL_8<P><S>
//    <P>: N (none), E (even), O (odd)
//    <S>: 1-2
//  Examples: SERIAL_8E1 (8 data bits, even parity, 1 stop bit), SERIAL_8O2 (8 data bits, odd parity, 2 stop bits)
#define CFG_MB_PARITY   1 // TODO

// User-defined temperature offset (°C): float value
#define CFG_TEMP_OFFSET     0.0


#define CFG_SNDEV_TIME_WEIGHTING SNDEV_TIME_WEIGHTING_SLOW
#define CFG_SNDEV_FREQ_WEIGHTING SNDEV_FREQ_WEIGHTING_A
