/*
  ExoSense.cpp - Exo Sense RP base library

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#include "ExoSense.h"
#include <Arduino.h>
#include <Wire.h>
#include "libs/includes.cpp"

ExoSenseClass::ExoSenseClass() {
}

void ExoSenseClass::setup() {
  pinMode(EXOS_PIN_DO1, OUTPUT);

  pinMode(EXOS_PIN_DI1, INPUT);
  pinMode(EXOS_PIN_DI2, INPUT);
  gpio_disable_pulls(EXOS_PIN_DI1);
  gpio_disable_pulls(EXOS_PIN_DI2);

  pinMode(EXOS_PIN_LED, OUTPUT);

  pinMode(EXOS_PIN_BUZZER, OUTPUT);

  pinMode(EXOS_PIN_PIR, INPUT);

  pinMode(EXOS_PIN_RS485_TXEN_N, OUTPUT);
  rs485TxEn(false);

  EXOS_RS485.setRX(EXOS_PIN_RS485_RX);
  EXOS_RS485.setTX(EXOS_PIN_RS485_TX);
  EXOS_RS485.begin(9600);
  EXOS_RS485.end();

  Wire.setSDA(EXOS_PIN_I2C_SDA);
  Wire.setSCL(EXOS_PIN_I2C_SCL);

  opt3001.begin(EXOS_I2C_ADDR_SENS_LIGHT);
  OPT3001_Config opt3001Cfg;
  opt3001Cfg.rawData = 0xCC10;
  opt3001.writeConfig(opt3001Cfg);

  sht40.begin(Wire);
  sgp40.begin(Wire);

  ics43432.setSCK(EXOS_PIN_I2S_SCK);
  ics43432.setWS(EXOS_PIN_I2S_WS);
  ics43432.setSD(EXOS_PIN_I2S_SD);

  lm75a_u9 = M2M_LM75A(EXOS_I2C_ADDR_SENS_SYS_TEMP_U9);
  lm75a_u16 = M2M_LM75A(EXOS_I2C_ADDR_SENS_SYS_TEMP_U16);

  rtc = RTCx(EXOS_I2C_ADDR_RTC, RTCx::MCP7941x);
}

void ExoSenseClass::rs485TxEn(bool enabled) {
  digitalWrite(EXOS_PIN_RS485_TXEN_N, enabled ? LOW : HIGH);
}

void ExoSenseClass::temperatureOffsetCompensate(float tempOffset,
          float* temperature, float* rh) {
  *temperature += tempOffset;
  *rh = *rh / pow(10, 0.032 * tempOffset);
  if (*rh > 100) {
    *rh = 100;
  }
}

bool ExoSenseClass::ics43432Begin(int bufferSize, long sampleRate) {
  if (!ics43432.setBufferSize(bufferSize)) {
    return false;
  }
  return ics43432.begin(I2S_MODE_MONO, sampleRate, ICS43432_SAMPLE_FRAME_BITS);
}

int32_t ExoSenseClass::ics43432Bytes2Sample(uint8_t* bytes) {
  int32_t sample = (unsigned int) bytes[3] * 65536
                 + (unsigned int) bytes[2] * 256
                 + (unsigned int) bytes[1];
  if ((sample & 0x800000) == 0x800000) {
    sample |= 0xff000000;
  }
  return sample;
}

ExoSenseClass ExoSense;
