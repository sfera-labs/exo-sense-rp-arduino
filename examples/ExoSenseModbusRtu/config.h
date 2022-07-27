// == Modbus unit address: 1-247 ==
#define CFG_MB_UNIT_ADDDR   1

// == Modbus baudrate (bit/s) ==
//#define CFG_MB_BAUDRATE     1 // 1200
//#define CFG_MB_BAUDRATE     2 // 2400
//#define CFG_MB_BAUDRATE     3 // 4800
//#define CFG_MB_BAUDRATE     4 // 9600
#define CFG_MB_BAUDRATE     5 // 19200
//#define CFG_MB_BAUDRATE     6 // 38400
//#define CFG_MB_BAUDRATE     7 // 57600
//#define CFG_MB_BAUDRATE     8 // 115200

// == Modbus serial parity and stop bits ==
#define CFG_MB_PARITY       1 // parity even, 1 stop bit
//#define CFG_MB_PARITY       2 // parity odd, 1 stop bit
//#define CFG_MB_PARITY       3 // parity none, 2 stop bits

// == Sound evaluation time weighting ==
#define CFG_SNDEV_TIME_WEIGHTING SNDEV_TIME_WEIGHTING_SLOW
//#define CFG_SNDEV_TIME_WEIGHTING SNDEV_TIME_WEIGHTING_FAST
//#define CFG_SNDEV_TIME_WEIGHTING SNDEV_TIME_WEIGHTING_IMPULSE

// == Sound evaluation frequency weighting ==
#define CFG_SNDEV_FREQ_WEIGHTING SNDEV_FREQ_WEIGHTING_A
//#define CFG_SNDEV_FREQ_WEIGHTING SNDEV_FREQ_WEIGHTING_C
//#define CFG_SNDEV_FREQ_WEIGHTING SNDEV_FREQ_WEIGHTING_Z

// == Temperature offset (°C/10): signed int value (-25 = -2.5°C) ==
#define CFG_TEMP_OFFSET     -25

// == Value to be written in modbus register to commit config ==
// (change it to invalidate currently saved configuration)
#define CFG_COMMIT_VAL      0xabcd


#include <EEPROM.h>

bool configReset = false;

void configCommit(uint16_t* data, uint8_t len) {
  byte checksum = len;
  for (int w = 0; w < len; w++) {
    for (int b = 0; b < 2; b++) {
      byte val = (data[w] >> (8 * b)) & 0xff;
      EEPROM.write(w * 2 + b + 2, val);
      checksum ^= val;
    }
  }
  EEPROM.write(0, len);
  EEPROM.write(1, checksum);
  EEPROM.commit();

  configReset = true;
}

bool configRead(uint16_t* data, uint8_t len) {
  EEPROM.begin(256);
  if (EEPROM.read(0) != len) {
    return false;
  }
  byte checksum = len;
  for (int w = 0; w < len; w++) {
    for (int b = 0; b < 2; b++) {
      byte val = EEPROM.read(w * 2 + b + 2);
      checksum ^= val;
      data[w] = data[w] | ((val << (8 * b)) & (0xff << (8 * b)));
    }
  }
  if (data[0] != CFG_COMMIT_VAL) {
    return false;
  }
  if (EEPROM.read(1) != checksum) {
    return false;
  }
  return true;
}
