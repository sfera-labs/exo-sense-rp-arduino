// Modbus unit address: 1-255
#define CFG_MB_UNIT_ADDDR   10

// Modbus baudrate (bit/s)
#define CFG_MB_BAUDRATE     19200

// Modbus serial configuration (data bits, parity, and stop bits)
//  Format: SERIAL_<B><P><S>
//    <B>: 5-8
//    <P>: N (none), E (even), O (odd)
//    <S>: 1-2
//  Examples: SERIAL_8E1 (8 data bits, even parity, 1 stop bit), SERIAL_7O2 (7 data bits, odd parity, 2 stop bits)
#define CFG_MB_SERIAL_CFG   SERIAL_8E1

// User-defined temperature offset (°C): float value
#define CFG_TEMP_OFFSET     0.0
