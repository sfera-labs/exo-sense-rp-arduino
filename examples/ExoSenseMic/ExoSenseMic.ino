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
 * Usage: upload the sketch, then open Tools > Serial Plotter
 *
 */

#include <ExoSense.h>

#define SAMPLING_RATE_HZ 44100

void setup() {
  Serial.begin(9600);
  ExoSense.setup();
  while (!Serial) ;
  if (!ExoSense.ics43432.begin(SAMPLING_RATE_HZ)) {
    Serial.println("Microphone begin error");
    while (true) ;
  }
  Serial.println("Ready");
}

void loop() {
  int sample = ExoSense.ics43432.read();
  Serial.println(sample);
}
