/*
 * ExoSenseSoundEval.ino - Using Exo Sense RP's microphone for sound evaluation
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

void setup() {
  bool ok;
  Serial.begin(9600);
  while (!Serial) ;
  
  ExoSense.setup();

  if (!ExoSense.ics43432.setBuffers(10, 1000)) {
    Serial.println("Microphone setup error");
    while (true) ;
  }

  if (!ExoSense.ics43432.begin(SNDEV_SAMPLING_FREQ_HZ)) {
    Serial.println("Microphone begin error");
    while (true) ;
  }

  if (!SoundEval.setMicSpecs(ICS43432_SENSITIVITY_DB, ICS43432_SAMPLE_VAL_MAX)) {
    Serial.println("Microphone specs error");
    while (true) ;
  }

  SoundEval.setPeriodResultCallback(onPeriodResult);

  /* Pick a time weighting */
  ok = SoundEval.setTimeWeighting(SNDEV_TIME_WEIGHTING_SLOW);
  // ok = SoundEval.setTimeWeighting(SNDEV_TIME_WEIGHTING_FAST);
  // ok = SoundEval.setTimeWeighting(SNDEV_TIME_WEIGHTING_IMPULSE);
  if (!ok) {
    Serial.println("Time Weighting error");
    while (true) ;
  }

  /* Pick a frequency weighting */
  ok = SoundEval.setFreqWeighting(SNDEV_FREQ_WEIGHTING_A);
  // ok = SoundEval.setFreqWeighting(SNDEV_FREQ_WEIGHTING_C);
  // ok = SoundEval.setFreqWeighting(SNDEV_FREQ_WEIGHTING_Z);
  if (!ok) {
    Serial.println("Frequency Weighting error");
    while (true) ;
  }

  Serial.println("Ready");
}

void loop() {
  SoundEval.process(ExoSense.ics43432.read());
}

void onPeriodResult(float lEqPeriodDb) {
  Serial.print("Period Leq: ");
  Serial.print(lEqPeriodDb);
  Serial.println(" dB");

  Serial.print("Period dominant frequency: ");
  Serial.print(SoundEval.getPeriodDominantFrequency());
  Serial.println(" Hz");
}
