#include <ModbusRtuSlave.h>

#define MB_REG_IN_START                300
#define MB_REG_IN_OFFSET_TEMP          1
#define MB_REG_IN_OFFSET_RH            2
#define MB_REG_IN_OFFSET_VOC_RAW       4
#define MB_REG_IN_OFFSET_VOC_IDX       5
#define MB_REG_IN_OFFSET_LUX           7
#define MB_REG_IN_OFFSET_PIR           9
#define MB_REG_IN_OFFSET_LEQ_PRD_MIN   11
#define MB_REG_IN_OFFSET_LEQ_PRD_MAX   12
#define MB_REG_IN_OFFSET_LEQ_PRD_AVG   13
#define MB_REG_IN_OFFSET_MAX           MB_REG_IN_OFFSET_LEQ_PRD_AVG

#define MB_REG_CFG_START               1000
#define MB_REG_CFG_OFFSET_COMMIT       0
#define MB_REG_CFG_OFFSET_MB_ADDR      1
#define MB_REG_CFG_OFFSET_MB_BAUD      2
#define MB_REG_CFG_OFFSET_MB_PARITY    3
#define MB_REG_CFG_OFFSET_SND_TIME     4
#define MB_REG_CFG_OFFSET_SND_FREQ     5
#define MB_REG_CFG_OFFSET_TMP_OFF      6
#define MB_REG_CFG_OFFSET_MAX          MB_REG_CFG_OFFSET_TMP_OFF

#define MB_REG_VAL_ERR UINT16_MAX

bool _buzzEnabled = false;
unsigned long _buzzStart;
unsigned long _buzzTime;

bool _doEnabled = false;
unsigned long _doStart;
unsigned long _doTime;

int16_t _leqPrdMin = INT16_MAX;
int16_t _leqPrdMax = INT16_MIN;
double _leqPrdAvg = 0;
uint32_t _leqPrdCnt = 0;
auto_init_mutex(_leqPrdMtx);

static word _inputRegisters[MB_REG_IN_OFFSET_MAX + 1];
static word _cfgRegisters[MB_REG_CFG_OFFSET_MAX + 1];

void modbusSetTemperature(float val) {
  val *= 10;
  if (val < INT16_MIN || val > INT16_MAX) {
    val = INT16_MIN;
  }
  _inputRegisters[MB_REG_IN_OFFSET_TEMP] = (int) val;
}

void modbusSetRh(float val) {
  val *= 10;
  if (val < 0 || val > 1000) {
    val = UINT16_MAX;
  }
  _inputRegisters[MB_REG_IN_OFFSET_RH] = (int) val;
}

void modbusSetVocRaw(uint16_t val) {
  _inputRegisters[MB_REG_IN_OFFSET_VOC_RAW] = val;
}

void modbusSetVocIdx(int32_t val) {
  if (val < 0 || val > 500) {
    val = UINT16_MAX;
  }
  _inputRegisters[MB_REG_IN_OFFSET_VOC_IDX] = val;
}

void modbusSetLux(float val) {
  val *= 10;
  if (val < 0 || val > UINT16_MAX) {
    val = UINT16_MAX;
  }
  _inputRegisters[MB_REG_IN_OFFSET_LUX] = (int) val;
}

void modbusSetLeqPrd(float val) {
  if (!mutex_enter_timeout_ms(&_leqPrdMtx, 3)) {
    return;
  }
  val *= 10;
  if (val >= INT16_MIN && val <= INT16_MAX) {
    if (val < _leqPrdMin) {
      _leqPrdMin = val;
    }
    if (val > _leqPrdMax) {
      _leqPrdMax = val;
    }
    _leqPrdAvg = _leqPrdAvg * _leqPrdCnt + val;
    _leqPrdAvg /= ++_leqPrdCnt;

    _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_MIN] = _leqPrdMin;
    _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_MAX] = _leqPrdMax;
    _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_AVG] = (int) _leqPrdAvg;
  }
  mutex_exit(&_leqPrdMtx);
}

void modbusPirIsr() {
  _inputRegisters[MB_REG_IN_OFFSET_PIR]++;
}

static bool checkAddrRange(word regAddr, word qty, word min, word max) {
  return regAddr >= min && regAddr <= max && regAddr + qty <= max + 1;
}

static uint8_t idxToDI(int i) {
  switch (i) {
    case 1:
      return EXOS_PIN_DI1;
    case 2:
      return EXOS_PIN_DI2;
    default:
      return -1;
  }
}

byte modbusOnRequest(byte unitAddr, byte function, word regAddr, word qty, byte *data) {
  switch (function) {
    case MB_FC_READ_DISCRETE_INPUTS:
      if (checkAddrRange(regAddr, qty, 101, 102)) {
        for (int i = regAddr - 100; i < regAddr - 100 + qty; i++) {
          ModbusRtuSlave.responseAddBit(digitalRead(idxToDI(i)));
        }
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_READ_COILS:
      if (regAddr == 201 && qty == 1) {
        ModbusRtuSlave.responseAddBit(digitalRead(EXOS_PIN_DO1));
        return MB_RESP_OK;

      } else if (regAddr == 501 && qty == 1) {
        ModbusRtuSlave.responseAddBit(digitalRead(EXOS_PIN_LED));
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_WRITE_SINGLE_COIL:
      if (regAddr == 201) {
        bool on = ModbusRtuSlave.getDataCoil(function, data, 0);
        digitalWrite(EXOS_PIN_DO1, on ? HIGH : LOW);
        return MB_RESP_OK;

      } else if (regAddr == 501) {
        bool on = ModbusRtuSlave.getDataCoil(function, data, 0);
        digitalWrite(EXOS_PIN_LED, on ? HIGH : LOW);
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_READ_INPUT_REGISTER:
      if (regAddr == 10 && qty == 1) {
        ModbusRtuSlave.responseAddRegister(EXOS_MODBUS_RTU_VERSION);
        return MB_RESP_OK;

      } else if (checkAddrRange(regAddr, qty, MB_REG_IN_START, MB_REG_IN_START + MB_REG_IN_OFFSET_MAX)) {
        int offset = regAddr - MB_REG_IN_START;
        int offsetEnd = offset + qty;

        bool rdLPMin = (MB_REG_IN_OFFSET_LEQ_PRD_MIN >= offset && MB_REG_IN_OFFSET_LEQ_PRD_MIN <= offsetEnd);
        bool rdLPMax = (MB_REG_IN_OFFSET_LEQ_PRD_MAX >= offset && MB_REG_IN_OFFSET_LEQ_PRD_MAX <= offsetEnd);
        bool rdLPAvg = (MB_REG_IN_OFFSET_LEQ_PRD_AVG >= offset && MB_REG_IN_OFFSET_LEQ_PRD_AVG <= offsetEnd);

        if (rdLPMin || rdLPMax || rdLPAvg) {
          if (!mutex_enter_timeout_ms(&_leqPrdMtx, 3)) {
            return MB_EX_SERVER_DEVICE_BUSY;
          }
        }

        for (int i = offset; i < offsetEnd; i++) {
          ModbusRtuSlave.responseAddRegister(_inputRegisters[i]);
        }

        // Reset LEQ registers when read
        if (rdLPMin) {
          _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_MIN] = INT16_MIN;
          _leqPrdMin = INT16_MAX;
        }
        if (rdLPMax) {
          _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_MAX] = INT16_MIN;
          _leqPrdMax = INT16_MIN;
        }
        if (rdLPAvg) {
          _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_AVG] = INT16_MIN;
          _leqPrdAvg = 0;
          _leqPrdCnt = 0;
        }

        if (rdLPMin || rdLPMax || rdLPAvg) {
          mutex_exit(&_leqPrdMtx);
        }

        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_READ_HOLDING_REGISTERS:
      if (checkAddrRange(regAddr, qty, MB_REG_CFG_START, MB_REG_CFG_START + MB_REG_CFG_OFFSET_MAX)) {
        int offset = regAddr - MB_REG_CFG_START;
        int offsetEnd = offset + qty;

        for (int i = offset; i < offsetEnd; i++) {
          ModbusRtuSlave.responseAddRegister(_cfgRegisters[i]);
        }

        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_WRITE_SINGLE_REGISTER:
    case MB_FC_WRITE_MULTIPLE_REGISTERS:
      if (regAddr == 211 && qty == 1) {
        _doTime = 100 * ModbusRtuSlave.getDataRegister(function, data, 0);
        if (_doTime > 0) {
          _doEnabled = true;
          _doStart = millis();
          digitalWrite(EXOS_PIN_DO1, HIGH);
        } else {
          digitalWrite(EXOS_PIN_DO1, LOW);
        }
        return MB_RESP_OK;

      } else if (regAddr == 401 && qty == 1) {
        _buzzTime = 100 * ModbusRtuSlave.getDataRegister(function, data, 0);
        if (_buzzTime > 0) {
          _buzzEnabled = true;
          _buzzStart = millis();
          digitalWrite(EXOS_PIN_BUZZER, HIGH);
        } else {
          digitalWrite(EXOS_PIN_BUZZER, LOW);
        }
        return MB_RESP_OK;

      } else if (checkAddrRange(regAddr, qty, MB_REG_CFG_START, MB_REG_CFG_START + MB_REG_CFG_OFFSET_MAX)) {
        int offset = regAddr - MB_REG_CFG_START;
        int offsetEnd = offset + qty;

        for (int i = offset; i < offsetEnd; i++) {
          word val = ModbusRtuSlave.getDataRegister(function, data, i - offset);
          switch (i) {
            case MB_REG_CFG_OFFSET_COMMIT:
              if (qty == 1) {
                if (val == CFG_COMMIT_VAL) {
                  configCommit(_cfgRegisters, MB_REG_CFG_OFFSET_MAX + 1);
                  return MB_RESP_OK;
                } else if (val == CFG_RESET_VAL) {
                  configResetToDefaults();
                  return MB_RESP_OK;
                }
              }
              return MB_EX_ILLEGAL_DATA_VALUE;
            case MB_REG_CFG_OFFSET_MB_ADDR:
              if (val < 1 || val > 247) {
                return MB_EX_ILLEGAL_DATA_VALUE;
              }
              break;
            case MB_REG_CFG_OFFSET_MB_BAUD:
              if (val < 1 || val > 8) {
                return MB_EX_ILLEGAL_DATA_VALUE;
              }
              break;
            case MB_REG_CFG_OFFSET_MB_PARITY:
            case MB_REG_CFG_OFFSET_SND_TIME:
            case MB_REG_CFG_OFFSET_SND_FREQ:
              if (val < 1 || val > 3) {
                return MB_EX_ILLEGAL_DATA_VALUE;
              }
              break;
          }
          _cfgRegisters[i] = val;
        }

        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    default:
      return MB_EX_ILLEGAL_FUNCTION;
  }
}

void modbusBegin(byte unitAddr, uint16_t baudIdx, uint16_t parity) {
  uint16_t serCfg;
  switch (parity) {
    case 2:
      serCfg = SERIAL_8O1;
      break;
    case 3:
      serCfg = SERIAL_8N2;
      break;
    default:
      serCfg = SERIAL_8E1;
  }

  unsigned long baud;
  switch (baudIdx) {
    case 1:
      baud = 1200;
      break;
    case 2:
      baud = 2400;
      break;
    case 3:
      baud = 4800;
      break;
    case 5:
      baud = 19200;
      break;
    case 6:
      baud = 38400;
      break;
    case 7:
      baud = 57600;
      break;
    case 8:
      baud = 115200;
      break;
    default:
      baud = 9600;
  }

  EXOS_RS485.begin(baud, serCfg);
  ModbusRtuSlave.setCallback(&modbusOnRequest);
  ModbusRtuSlave.begin(unitAddr, &EXOS_RS485, baud, EXOS_PIN_RS485_TXEN_N, true);

  _inputRegisters[MB_REG_IN_OFFSET_TEMP] = INT16_MIN;
  _inputRegisters[MB_REG_IN_OFFSET_RH] = UINT16_MAX;
  _inputRegisters[MB_REG_IN_OFFSET_VOC_RAW] = UINT16_MAX;
  _inputRegisters[MB_REG_IN_OFFSET_VOC_IDX] = UINT16_MAX;
  _inputRegisters[MB_REG_IN_OFFSET_LUX] = UINT16_MAX;
  _inputRegisters[MB_REG_IN_OFFSET_PIR] = 0;
  _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_MIN] = INT16_MIN;
  _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_MAX] = INT16_MIN;
  _inputRegisters[MB_REG_IN_OFFSET_LEQ_PRD_AVG] = INT16_MIN;
}

void modbusProcess() {
  ModbusRtuSlave.process();
}
