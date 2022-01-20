/*
 * ExoSenseLED.ino - Using Exo Sense RP's LED light
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
  Serial.begin(9600);
  ExoSense.setup();
  while (!Serial) ;
}

void loop() {
  int led = digitalRead(EXOS_PIN_LED);
  
  Serial.print("LED ");
  Serial.println(led ? "ON" : "OFF");
  
  delay(1000);
  
  // Toggle LED
  digitalWrite(EXOS_PIN_LED, !led);
}
