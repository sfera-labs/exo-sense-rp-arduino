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

#include <ExoSense.h>
#include "modbus.h"
#include "config.h"

unsigned long lastReadTs = 0;

void setup() {
  Serial.begin(9600);
  
  ExoSense.setup();
  
  attachInterrupt(digitalPinToInterrupt(EXOS_PIN_PIR), modbusPirIsr, RISING);
  
  modbusBegin(CFG_MB_UNIT_ADDDR, CFG_MB_BAUDRATE, CFG_MB_SERIAL_CFG);

  // TODO remove ====
  //while(!Serial);
  //Serial.println("ready");
  // ================
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
    Serial.print("SHT40 error: ");
    Serial.println(error);
    
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
    Serial.print("SGP40 error: ");
    Serial.println(error);
    
    modbusSetVocRaw(MB_REG_VAL_ERR);
  } else {
    modbusSetVocRaw(srawVoc);
    int32_t vocIdx = ExoSense.voc.process(srawVoc);
    modbusSetVocIdx(vocIdx);
  }
}
