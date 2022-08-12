/*
 * ExoSenseModbusRtu.ino -
 *   Using Exo Sense RP as a Modbus RTU slave unit
 *
 *   Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.
 *
 *   For information, see:
 *   http://www.sferalabs.cc/
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See file LICENSE.txt for further informations on licensing terms.
 *
 */

#define EXOS_MODBUS_RTU_VERSION 0x0300

#include <ExoSense.h>
#include "config.h"
#include "modbus.h"
#include "hardware/watchdog.h"


// == Core 1: Modbus, sensors, I/O ================

unsigned long lastReadTs = 0;

void setup() {
  watchdog_enable(2000, 1);

  ExoSense.setup();
  loadConfig();
  attachInterrupt(digitalPinToInterrupt(EXOS_PIN_PIR), modbusPirIsr, RISING);
  modbusBegin(_cfgRegisters[MB_REG_CFG_OFFSET_MB_ADDR],
              _cfgRegisters[MB_REG_CFG_OFFSET_MB_BAUD],
              _cfgRegisters[MB_REG_CFG_OFFSET_MB_PARITY]);

  watchdog_update();

  digitalWrite(EXOS_PIN_BUZZER, HIGH);

  // Signal to other core setup done
  rp2040.fifo.push_nb(0);
  rp2040.fifo.push_nb(1);
}

void loop() {
  modbusProcess();

  if (configReset) {
    // let watchdog expire
    while(true) {
      delay(100);
    }
  }

  if (millis() - lastReadTs > 1000) {
    readTempRhVoc();
    readLux();
    lastReadTs = millis();
  }

  if (_buzzEnabled && millis() - _buzzStart > _buzzTime) {
    digitalWrite(EXOS_PIN_BUZZER, LOW);
    _buzzEnabled = false;
  }

  if (_doEnabled && millis() - _doStart > _doTime) {
    digitalWrite(EXOS_PIN_DO1, LOW);
    _doEnabled = false;
  }

  // ping-pong with other core to update watchdog
  uint32_t popped;
  if (rp2040.fifo.pop_nb(&popped) && popped == 2) {
    rp2040.fifo.push_nb(1);
    watchdog_update();
  }
}

void loadConfig() {
  if (!configRead(_cfgRegisters, MB_REG_CFG_OFFSET_MAX + 1)) {
    _cfgRegisters[MB_REG_CFG_OFFSET_MB_ADDR] = CFG_MB_UNIT_ADDDR;
    _cfgRegisters[MB_REG_CFG_OFFSET_MB_BAUD] = CFG_MB_BAUDRATE;
    _cfgRegisters[MB_REG_CFG_OFFSET_MB_PARITY] = CFG_MB_PARITY;
    _cfgRegisters[MB_REG_CFG_OFFSET_SND_TIME] = CFG_SNDEV_TIME_WEIGHTING;
    _cfgRegisters[MB_REG_CFG_OFFSET_SND_FREQ] = CFG_SNDEV_FREQ_WEIGHTING;
    _cfgRegisters[MB_REG_CFG_OFFSET_TMP_OFF] = CFG_TEMP_OFFSET;
  }
  _cfgRegisters[MB_REG_CFG_OFFSET_COMMIT] = CFG_COMMIT_VAL;
}

void readLux() {
  OPT3001 result = ExoSense.opt3001.readResult();
  if (result.error == NO_ERROR) {
    modbusSetLux(result.lux);
  } else {
    modbusSetLux(MB_REG_VAL_ERR);
  }
}

void readTempRhVoc() {
  float humidity;
  float temperature;

  uint16_t error = ExoSense.sht40.measureHighPrecision(temperature, humidity);
  if (error) {
    modbusSetRh(MB_REG_VAL_ERR);
    modbusSetTemperature(MB_REG_VAL_ERR);
    humidity = SGP40_DEFAULT_HUMIDITY;
    temperature = SGP40_DEFAULT_TEMPERATURE;
  } else {
    if (_cfgRegisters[MB_REG_CFG_OFFSET_TMP_OFF] != 0) {
      float userCompTemp = temperature;
      float userCompRh = humidity;
      float tempOffset = ((int16_t) _cfgRegisters[MB_REG_CFG_OFFSET_TMP_OFF]) / 10.0;
      ExoSense.temperatureOffsetCompensate(
                  tempOffset, &userCompTemp, &userCompRh);
      modbusSetRh(userCompRh);
      modbusSetTemperature(userCompTemp);
    } else {
      modbusSetRh(humidity);
      modbusSetTemperature(temperature);
    }
  }

  uint16_t srawVoc;
  uint16_t compensationRh = SHT40_TICKS_FROM_PERCENT_RH(humidity);
  uint16_t compensationT = SHT40_TICKS_FROM_CELSIUS(temperature);
  error = ExoSense.sgp40.measureRawSignal(
                          compensationRh, compensationT, srawVoc);
  if (error) {
    modbusSetVocRaw(MB_REG_VAL_ERR);
  } else {
    modbusSetVocRaw(srawVoc);
    int32_t vocIdx = ExoSense.voc.process(srawVoc);
    modbusSetVocIdx(vocIdx);
  }
}

// == Core 2: Sound eval ================

unsigned long micStartTs;
bool micReady;
int micSample;

void setup1() {
  micReady = false;

  // Wait for other core to setup
  rp2040.fifo.pop();

  ExoSense.ics43432.setBuffers(10, 1000);
  ExoSense.ics43432.begin(SNDEV_SAMPLING_FREQ_HZ);
  SoundEval.setMicSpecs(ICS43432_SENSITIVITY_DB, ICS43432_SAMPLE_VAL_MAX);
  SoundEval.setPeriodResultCallback(onSoundEvalResult);
  SoundEval.setTimeWeighting(_cfgRegisters[MB_REG_CFG_OFFSET_SND_TIME]);
  SoundEval.setFreqWeighting(_cfgRegisters[MB_REG_CFG_OFFSET_SND_FREQ]);

  digitalWrite(EXOS_PIN_BUZZER, LOW);

  micStartTs = millis();
}

void loop1() {
  // ping-pong with other core to update watchdog
  uint32_t popped;
  if (rp2040.fifo.pop_nb(&popped) && popped == 1) {
    rp2040.fifo.push_nb(2);
  }

  micSample = ExoSense.ics43432.read();
  if (!micReady) {
    // discard initial noise readings
    if (millis() - micStartTs > 2000) {
       micReady = true;
    }
    return;
  }
  SoundEval.process(micSample);
}

void onSoundEvalResult(float lEqPeriodDb) {
  modbusSetLeqPrd(lEqPeriodDb);
}
