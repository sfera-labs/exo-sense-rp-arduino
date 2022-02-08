/*
 * ExoSenseTempRhVoc.ino -
 *   Using Exo Sense RP's temperature, humidity and VOC sensors
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
 * The ExoSense.sht40 and ExoSense.sgp40 objects are respectively
 * instaces of the SensirionI2CSht4x and SensirionI2CSgp40 classes
 * from the Sensirion's arduino-i2c-sht4x and arduino-i2c-sgp40
 * libraries, included in the ExoSense library.
 *
 * For examples using other features and cofigurations:
 * https://github.com/sfera-labs/arduino-i2c-sht4x
 * https://github.com/sfera-labs/arduino-i2c-sgp40
 * https://github.com/sfera-labs/arduino-gas-index-algorithm
 *
 */

#include <ExoSense.h>

unsigned long sgpReadTs = 0;

// User-defined temperature offset
float tempOffset = -3.5;

void setup() {
  Serial.begin(9600);
  ExoSense.setup();
  while (!Serial) ;
  Serial.println("Ready");
}

void loop() {
  float humidity;
  float temperature;

  Serial.println("----");

  uint16_t error = ExoSense.sht40.measureHighPrecision(temperature, humidity);
  if (error) {
    Serial.print("SHT40 error: ");
    Serial.println(error);

    humidity = SGP40_DEFAULT_HUMIDITY;
    temperature = SGP40_DEFAULT_TEMPERATURE;
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");

    float userCompTemp = temperature;
    float userCompRh = humidity;
    ExoSense.temperatureOffsetCompensate(
                tempOffset, &userCompTemp, &userCompRh);
    Serial.print("Temperature comp: ");
    Serial.print(userCompTemp, 2);
    Serial.println(" C");
    Serial.print("Humidity comp: ");
    Serial.print(userCompRh, 2);
    Serial.println(" %");
  }

  // VOX index algorithm sampling interval = 1s
  if (millis() - sgpReadTs > 1000) {
    uint16_t srawVoc;
    uint16_t compensationRh = SHT40_TICKS_FROM_PERCENT_RH(humidity);
    uint16_t compensationT = SHT40_TICKS_FROM_CELSIUS(temperature);
    error = ExoSense.sgp40.measureRawSignal(
                            compensationRh, compensationT, srawVoc);
    if (error) {
      Serial.print("SGP40 error: ");
      Serial.println(error);
    } else {
      int32_t vocIdx = ExoSense.voc.process(srawVoc);
      Serial.print("VOC index: ");
      Serial.print(vocIdx);
      Serial.println();
    }

    sgpReadTs = millis();
  }

  delay(500);
}
