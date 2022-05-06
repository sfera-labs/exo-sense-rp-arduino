/*
 * ExoSenseRtc.ino - Using Exo Sense RP's RTC
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
 * The ExoSense.rtc object is an instace of the RTCx class from
 * Steve Marple's RTCx library, included in the ExoSense library.
 *
 * For other examples:
 * https://github.com/sfera-labs/arduino-RTCx
 *
 */

#include <ExoSense.h>

#define ISO_TIME_FMT_LEN 20 // Format: 2001-01-01T00:00:22

char time[ISO_TIME_FMT_LEN];
unsigned long lastSet;

void setup() {
  Serial.begin(9600);
  ExoSense.setup();
  while (!Serial) ;
  Serial.println("Ready");

  ExoSense.rtc.enableBatteryBackup();

  lastSet = millis();
}

void loop() {
  ExoSense.rtc.readClock(time, ISO_TIME_FMT_LEN);
  Serial.println(time);

  if (millis() - lastSet > 10000) {
    Serial.println("Setting time...");
    ExoSense.rtc.setClock("2020-02-08T10:15:27");
    lastSet = millis();
  }

  delay(1000);
}
