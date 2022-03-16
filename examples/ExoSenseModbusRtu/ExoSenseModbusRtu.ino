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

// TODO ADD WATCHDOG LOGIC !!!!!

#include <ExoSense.h>
#include "modbus.h"
#include "config.h"

#define MIC_BUFF_SIZE (1000 * ICS43432_BYTES_PER_SAMPLE_FRAME)
#define I2S_INTERNAL_BUFFER_SIZE (MIC_BUFF_SIZE * 10)

unsigned long lastReadTs = 0;

void setup() {
  Serial.begin(9600);

  ExoSense.setup();

  attachInterrupt(digitalPinToInterrupt(EXOS_PIN_PIR), modbusPirIsr, RISING);
  modbusBegin(CFG_MB_UNIT_ADDDR, CFG_MB_BAUDRATE, CFG_MB_SERIAL_CFG);

  // TODO remove ====
  while(!Serial);
  Serial.println("ready I");
  // Serial.println(ARDUINO_PICO_VERSION_STR);
  // Serial.println(ARDUINO_PICO_MAJOR);
  // Serial.println(ARDUINO_PICO_MINOR);
  // ================
  
  // Signal to core 2 setup done
  rp2040.fifo.push_nb(0);

  // setup1x(); // TODO remove
}

void loop() {
  modbusProcess();

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

  // loop1x(); // TODO remove
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
    if (CFG_TEMP_OFFSET != 0) {
      float userCompTemp = temperature;
      float userCompRh = humidity;
      ExoSense.temperatureOffsetCompensate(
                  CFG_TEMP_OFFSET, &userCompTemp, &userCompRh);
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

// == Core 2 =====

uint8_t micBuff[MIC_BUFF_SIZE];
unsigned long micStartTs;
bool micReady;

void setup1() {
  micReady = false;

  // Wait for core 1 to setup
  rp2040.fifo.pop();

  // TODO remove ====
  while(!Serial);
  Serial.println("*********** setup1 1 ************");
  // ================

  ExoSense.ics43432Begin(I2S_INTERNAL_BUFFER_SIZE, SNDEV_SAMPLING_FREQ_HZ);
  SoundEval.setMicSpecs(ICS43432_SENSITIVITY_DB, ICS43432_SAMPLE_VAL_MAX);
  SoundEval.setPeriodResultCallback(onSoundEvalResult);
  SoundEval.setTimeWeighting(CFG_SNDEV_TIME_WEIGHTING);
  SoundEval.setFreqWeighting(CFG_SNDEV_FREQ_WEIGHTING);

  micStartTs = millis();
  
  // TODO remove ====
  Serial.println("*********** setup1 2 ************");
  // ================
}

void loop1() {
  digitalWrite(EXOS_PIN_LED, !digitalRead(EXOS_PIN_LED)); // TODO remove
  
  int ret = ExoSense.ics43432.read(micBuff, MIC_BUFF_SIZE);
  if (!micReady) {
    // discard initial noise readings
    if (millis() - micStartTs > 2000) {
       micReady = true; 
    }
    return;
  }
  if (ret > 0) {
    for (int i = 0; i < ret; i += ICS43432_BYTES_PER_SAMPLE_FRAME) {
      int32_t sample = ExoSense.ics43432Bytes2Sample(&micBuff[i]);
      SoundEval.process(sample);
    }
  } 
  // TODO remove =============
  else {
    Serial.print("Microphone read error: ");
    Serial.println(ret);
  }
  // =========================
}

void onSoundEvalResult(float lEqPeriodDb) {
  modbusSetLeqPrd(lEqPeriodDb);
  // Serial.println(lEqPeriodDb); // TODO remove
}
