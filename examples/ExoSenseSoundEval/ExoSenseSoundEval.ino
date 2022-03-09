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
#include "soundEval.h"

#define BUFF_SIZE (1000 * ICS43432_BYTES_PER_SAMPLE_FRAME)
#define I2S_INTERNAL_BUFFER_SIZE (BUFF_SIZE * 10)

uint8_t buffBytes[BUFF_SIZE];

void setup() {
  bool ok;
  Serial.begin(9600);
  while (!Serial) ;
  
  ExoSense.setup();
  
  if (!ExoSense.ics43432Begin(I2S_INTERNAL_BUFFER_SIZE, SNDEV_SAMPLING_FREQ_HZ)) {
    Serial.println("Microphone setup error");
    while (true) ;
  }

  if (!soundEvalSetMicSpecs(ICS43432_SENSITIVITY_DB, ICS43432_SAMPLE_VAL_MAX)) {
    Serial.println("Microphone specs error");
    while (true) ;
  }

  /* Pick a time weighting */
  ok = soundEvalSetTimeWeighting(SNDEV_TIME_WEIGHTING_SLOW);
  // ok = soundEvalSetTimeWeighting(SNDEV_TIME_WEIGHTING_FAST);
  // ok = soundEvalSetTimeWeighting(SNDEV_TIME_WEIGHTING_IMPULSE);
  if (!ok) {
    Serial.println("Time Weighting error");
    while (true) ;
  }

  /* Pick a frequency weighting */
  // ok = soundEvalSetFreqWeighting(SNDEV_FREQ_WEIGHTING_A);
  // ok = soundEvalSetFreqWeighting(SNDEV_FREQ_WEIGHTING_C);
  ok = soundEvalSetFreqWeighting(SNDEV_FREQ_WEIGHTING_Z);
  if (!ok) {
    Serial.println("Frequency Weighting error");
    while (true) ;
  }

  Serial.println("Ready");
}

void loop() {
  int ret = ExoSense.ics43432.read(buffBytes, BUFF_SIZE);
  if (ret > 0) {
    for (int i = 0; i < ret; i += ICS43432_BYTES_PER_SAMPLE_FRAME) {
      int32_t sample = ExoSense.ics43432Bytes2Sample(&buffBytes[i]);
      soundEvalProcess(sample);
    }
  } else {
    Serial.print("Microphone read error: ");
    Serial.println(ret);
  }
}
