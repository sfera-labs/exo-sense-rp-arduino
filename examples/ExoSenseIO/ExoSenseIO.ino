/*
 * ExoSenseIO.ino - Using I/Os on Exo Sense RP
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
 * Usage:
 *   - Set TTL2/DI2 input in TTL mode by moving the corresponding 
 *     internal jumper in BYP position
 *   - Connect DO1 to TTL2/DI2
 *   - Upload this sketch to Exo Sense and open the Serial Monitor
 *   - Every second the I/Os state is printed and DO1 is toggled
 * 
 * TTL inputs are pulled up, therefore HIGH by default, while DO1,
 * being an open collector, is floating by default and connected to
 * GND when HIGH.
 * Every time DO1 is set HIGH, TTL2 falls to GND, the interrupt
 * function is called and the counter increased.
 * 
 * DI inputs are pulled down, therefore LOW by default. Connect DI1
 * to VS+ to see its state change.
 * 
 * WARNING: Do NOT connect inputs in TTL mode to VS+
 * 
 */

#include <ExoSense.h>

int ttlCnt = 0;

void setup() {
  Serial.begin(9600);
  
  ExoSense.setup();
  
  pinMode(EXOS_PIN_TTL2, INPUT);
  attachInterrupt(digitalPinToInterrupt(EXOS_PIN_TTL2), ttlCntUp, FALLING);
  
  while (!Serial) ;
}

void loop() {
  int di1 = digitalRead(EXOS_PIN_DI1);
  int do1 = digitalRead(EXOS_PIN_DO1);
  int ttl2 = digitalRead(EXOS_PIN_TTL2);
  
  Serial.print("DI1: ");
  Serial.println(di1);
  
  Serial.print("DO1: ");
  Serial.println(do1);
  
  Serial.print("TTL2: ");
  Serial.println(ttl2);
  
  Serial.print("TTL2 count: ");
  Serial.println(ttlCnt);
  Serial.println();

  // Toggle DO1
  digitalWrite(EXOS_PIN_DO1, !do1);
  
  delay(1000);
}

void ttlCntUp() {
  ttlCnt++;
}
