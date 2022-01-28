/*
 * ExoSenseMic.ino - Using Exo Sense RP's microphone
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

#define BUFF_LEN 10000

uint8_t buff[BUFF_LEN];

void setup() {
  Serial.begin(9600);
  ExoSense.setup();
  while (!Serial) ;
  ExoSense.ics43432.setBufferSize(40000);
  if (!ExoSense.ics43432.begin(I2S_MODE_MONO, 44100, ICS43432_SAMPLE_BITS)) {
    Serial.println("Microphone setup error");
    while (true) ;
  }
  Serial.println("Ready");
}

void loop() {
  int ret = ExoSense.ics43432.read(buff, BUFF_LEN);
  if (ret < 0) {
    Serial.print("Microphone read error: ");
    Serial.println(ret);
    delay(1000);
    return;
  }
  Serial.print("Read ");
  Serial.print(ret);
  Serial.println(" bytes");
}
