/*
 * ExoSenseIO.ino - Using Exo Sense RP's PIR motion sensors
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

unsigned int pirCnt = 0;
unsigned int pirCntPrint = 0;

void setup() {
  Serial.begin(9600);
  ExoSense.setup();
  attachInterrupt(digitalPinToInterrupt(EXOS_PIN_PIR), pirCntUp, RISING);
  while (!Serial) ;
  Serial.println("Ready - Move!");
}

void loop() {
  if (pirCntPrint != pirCnt) {
    pirCntPrint = pirCnt;
    Serial.println("Motion detected");
  }
  
  delay(50);
}

void pirCntUp() {
  pirCnt++;
}
