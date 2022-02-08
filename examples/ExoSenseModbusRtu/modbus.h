#include <ModbusRtuSlave.h>

#define IN_REG_START                300
#define IN_REG_OFFSET_TEMP          1
#define IN_REG_OFFSET_RH            2
#define IN_REG_OFFSET_VOC_RAW       4
#define IN_REG_OFFSET_VOC_IDX       5
#define IN_REG_OFFSET_LUX           7
#define IN_REG_OFFSET_PIR           9
#define IN_REG_OFFSET_LEQ_ITV_MIN   11
#define IN_REG_OFFSET_LEQ_ITV_MAX   12
#define IN_REG_OFFSET_LEQ_ITV_AVG   13
#define IN_REG_OFFSET_LEQ_PRD_MIN   15
#define IN_REG_OFFSET_LEQ_PRD_MAX   16
#define IN_REG_OFFSET_LEQ_PRD_AVG   17
#define IN_REG_OFFSET_MAX           IN_REG_OFFSET_LEQ_PRD_AVG

#define MB_REG_VAL_ERR UINT16_MAX

bool _buzzEnabled = false;
unsigned long _buzzStart;
unsigned long _buzzTime;

bool _doEnabled = false;
unsigned long _doStart;
unsigned long _doTime;

static word _inputRegisters[IN_REG_OFFSET_MAX + 1];

int16_t _leqItvMin = INT16_MAX;
int16_t _leqItvMax = INT16_MIN;
double _leqItvAvg = 0;
uint32_t _leqItvCnt = 0;

int16_t _leqPrdMin = INT16_MAX;
int16_t _leqPrdMax = INT16_MIN;
double _leqPrdAvg = 0;
uint32_t _leqPrdCnt = 0;

void modbusSetTemperature(float val) {
  val *= 10;
  if (val < INT16_MIN || val > INT16_MAX) {
    val = INT16_MIN;
  }
  _inputRegisters[IN_REG_OFFSET_TEMP] = (int) val;
}

void modbusSetRh(float val) {
  val *= 10;
  if (val < 0 || val > 1000) {
    val = UINT16_MAX;
  }
  _inputRegisters[IN_REG_OFFSET_RH] = (int) val;
}

void modbusSetVocRaw(uint16_t val) {
  _inputRegisters[IN_REG_OFFSET_VOC_RAW] = val;
}

void modbusSetVocIdx(int32_t val) {
  if (val < 0 || val > 500) {
    val = UINT16_MAX;
  }
  _inputRegisters[IN_REG_OFFSET_VOC_IDX] = val;
}

void modbusSetLux(float val) {
  val *= 10;
  if (val < 0 || val > UINT16_MAX) {
    val = UINT16_MAX;
  }
  _inputRegisters[IN_REG_OFFSET_LUX] = (int) val;
}

void modbusSetLeqItv(float val) {
  val *= 10;
  if (val >= INT16_MIN && val <= INT16_MAX) {
    if (val < _leqItvMin) {
      _leqItvMin = val;
    }
    if (val > _leqItvMax) {
      _leqItvMax = val;
    }
    _leqItvAvg = _leqItvAvg * _leqItvCnt + val;
    _leqItvAvg /= ++_leqItvCnt;

    _inputRegisters[IN_REG_OFFSET_LEQ_ITV_MIN] = _leqItvMin;
    _inputRegisters[IN_REG_OFFSET_LEQ_ITV_MAX] = _leqItvMax;
    _inputRegisters[IN_REG_OFFSET_LEQ_ITV_AVG] = (int) _leqItvAvg;
  }
}

void modbusSetLeqPrd(float val) {
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

    _inputRegisters[IN_REG_OFFSET_LEQ_PRD_MIN] = _leqPrdMin;
    _inputRegisters[IN_REG_OFFSET_LEQ_PRD_MAX] = _leqPrdMax;
    _inputRegisters[IN_REG_OFFSET_LEQ_PRD_AVG] = (int) _leqPrdAvg;
  }
}

void modbusPirIsr() {
  _inputRegisters[IN_REG_OFFSET_PIR]++;
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
      if (checkAddrRange(regAddr, qty, IN_REG_START, IN_REG_START + IN_REG_OFFSET_MAX)) {
        int offset = regAddr - IN_REG_START;
        for (int i = offset; i < offset + qty; i++) {
          ModbusRtuSlave.responseAddRegister(_inputRegisters[i]);

          // Reset LEQ registers when read
          if (i == IN_REG_OFFSET_LEQ_ITV_MIN) {
            _inputRegisters[IN_REG_OFFSET_LEQ_ITV_MIN] = INT16_MIN;
            _leqItvMin = INT16_MAX;
          }
          if (i == IN_REG_OFFSET_LEQ_ITV_MAX) {
            _inputRegisters[IN_REG_OFFSET_LEQ_ITV_MAX] = INT16_MIN;
            _leqItvMax = INT16_MIN;
          }
          if (i == IN_REG_OFFSET_LEQ_ITV_AVG) {
            _inputRegisters[IN_REG_OFFSET_LEQ_ITV_AVG] = INT16_MIN;
            _leqItvAvg = 0;
            _leqItvCnt = 0;
          }

          if (i == IN_REG_OFFSET_LEQ_PRD_MIN) {
            _inputRegisters[IN_REG_OFFSET_LEQ_PRD_MIN] = INT16_MIN;
            _leqPrdMin = INT16_MAX;
          }
          if (i == IN_REG_OFFSET_LEQ_PRD_MAX) {
            _inputRegisters[IN_REG_OFFSET_LEQ_PRD_MAX] = INT16_MIN;
            _leqPrdMax = INT16_MIN;
          }
          if (i == IN_REG_OFFSET_LEQ_PRD_AVG) {
            _inputRegisters[IN_REG_OFFSET_LEQ_PRD_AVG] = INT16_MIN;
            _leqPrdAvg = 0;
            _leqPrdCnt = 0;
          }
        }
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_WRITE_SINGLE_REGISTER:
      if (regAddr == 211) {
        _doTime = 100 * ModbusRtuSlave.getDataRegister(function, data, 0);
        if (_doTime > 0) {
          _doEnabled = true;
          _doStart = millis();
          digitalWrite(EXOS_PIN_DO1, HIGH);
        } else {
          digitalWrite(EXOS_PIN_DO1, LOW);
        }
        return MB_RESP_OK;
      
      } else if (regAddr == 401) {
        _buzzTime = 100 * ModbusRtuSlave.getDataRegister(function, data, 0);
        if (_buzzTime > 0) {
          _buzzEnabled = true;
          _buzzStart = millis();
          digitalWrite(EXOS_PIN_BUZZER, HIGH);
        } else {
          digitalWrite(EXOS_PIN_BUZZER, LOW);
        }
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;
      
    default:
      return MB_EX_ILLEGAL_FUNCTION;
  }
}

void modbusBegin(byte unitAddr, unsigned long baud, unsigned long config) {
  EXOS_RS485.begin(baud, config);
  ModbusRtuSlave.setCallback(&modbusOnRequest);
  ModbusRtuSlave.begin(unitAddr, &EXOS_RS485, baud, EXOS_PIN_RS485_TXEN_N, true);

  _inputRegisters[IN_REG_OFFSET_TEMP] = INT16_MIN;
  _inputRegisters[IN_REG_OFFSET_RH] = UINT16_MAX;
  _inputRegisters[IN_REG_OFFSET_VOC_RAW] = UINT16_MAX;
  _inputRegisters[IN_REG_OFFSET_VOC_IDX] = UINT16_MAX;
  _inputRegisters[IN_REG_OFFSET_LUX] = UINT16_MAX;
  _inputRegisters[IN_REG_OFFSET_PIR] = 0;
  _inputRegisters[IN_REG_OFFSET_LEQ_ITV_MIN] = INT16_MIN;
  _inputRegisters[IN_REG_OFFSET_LEQ_ITV_MAX] = INT16_MIN;
  _inputRegisters[IN_REG_OFFSET_LEQ_ITV_AVG] = INT16_MIN;
  _inputRegisters[IN_REG_OFFSET_LEQ_PRD_MIN] = INT16_MIN;
  _inputRegisters[IN_REG_OFFSET_LEQ_PRD_MAX] = INT16_MIN;
  _inputRegisters[IN_REG_OFFSET_LEQ_PRD_AVG] = INT16_MIN;
}

void modbusProcess() {
  ModbusRtuSlave.process();
}
