/*
  ExoSense.h - Exo Sense RP base library

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef ExoSense_h
#define ExoSense_h

#include <I2S.h>
#include "libs/includes.h"

#define EXOS_PIN_DI1 28
#define EXOS_PIN_DI2 29
#define EXOS_PIN_TTL1 26
#define EXOS_PIN_TTL2 27
#define EXOS_PIN_DO1 10
#define EXOS_PIN_LED_BLUE 14
#define EXOS_PIN_LED_GREEN_N 11
#define EXOS_PIN_LED EXOS_PIN_LED_BLUE
#define EXOS_PIN_BUZZER 13
#define EXOS_PIN_BUZZER_PWM 12
#define EXOS_PIN_PIR 23
#define EXOS_PIN_RS485_TX 16
#define EXOS_PIN_RS485_RX 17
#define EXOS_PIN_RS485_TXEN_N 15
#define EXOS_PIN_I2S_SCK 6
#define EXOS_PIN_I2S_WS 7
#define EXOS_PIN_I2S_SD 8
#define EXOS_PIN_I2C_SDA 0
#define EXOS_PIN_I2C_SCL 1

#define EXOS_I2C_ADDR_SENS_TEMP_RH 0x44
#define EXOS_I2C_ADDR_SENS_LIGHT 0x45
#define EXOS_I2C_ADDR_SENS_SYS_TEMP_U9 0x48
#define EXOS_I2C_ADDR_SENS_SYS_TEMP_U16 0x49
#define EXOS_I2C_ADDR_SENS_VIBR 0x55
#define EXOS_I2C_ADDR_SENS_VOC 0x59
#define EXOS_I2C_ADDR_SECELEM 0x60
#define EXOS_I2C_ADDR_RTC 0x6f
#define EXOS_I2C_ADDR_RTC_EEPROM 0x57
#define EXOS_I2C_ADDR_EERAM_SRAM 0x50
#define EXOS_I2C_ADDR_EERAM_CTRL 0x18

#define SERIAL_PORT_MONITOR Serial
#define SERIAL_PORT_HARDWARE Serial1

#define EXOS_RS485 SERIAL_PORT_HARDWARE

#define SGP40_DEFAULT_HUMIDITY (50.0)
#define SGP40_DEFAULT_TEMPERATURE (25.0)

#define SHT40_TICKS_FROM_PERCENT_RH(h) static_cast<uint16_t>(h * 65535 / 100)
#define SHT40_TICKS_FROM_CELSIUS(t) static_cast<uint16_t>((t + 45) * 65535 / 175)

#define ICS43432_SAMPLE_FRAME_BITS (32)
#define ICS43432_SAMPLE_DATA_BITS (24)
#define ICS43432_SAMPLE_VAL_MAX (0x7FFFFF)
#define ICS43432_BYTES_PER_SAMPLE_FRAME (ICS43432_SAMPLE_FRAME_BITS / 8)
#define ICS43432_SENSITIVITY_DB (-26)

class I2SW: public I2S {
  using I2S::I2S;
  public:
    bool begin(long);
    int read();
};

class ExoSenseClass {
  public:
    ClosedCube_OPT3001 opt3001;
    SensirionI2CSht4x sht40;
    SensirionI2CSgp40 sgp40;
    VOCGasIndexAlgorithm voc;
    I2SW ics43432;
    M2M_LM75A lm75a_u9;
    M2M_LM75A lm75a_u16;
    RTCx rtc;

    ExoSenseClass();
    void setup();
    void rs485TxEn(bool enabled);
    void temperatureOffsetCompensate(float tempOffset,
              float* temperature, float* rh);
};

extern ExoSenseClass ExoSense;

#endif
