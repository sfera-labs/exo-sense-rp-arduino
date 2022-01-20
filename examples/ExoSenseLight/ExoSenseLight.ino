/*
 * ExoSenseLight.ino - Using Exo Sense RP's light sensor
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
 * The ExoSense.light object is an instace of the OPT3001 class from
 * the ClosedCube_OPT3001 library, included in the ExoSense library.
 * 
 * For examples using advanced OPT3001 features and cofigurations:
 * https://github.com/sfera-labs/ClosedCube_OPT3001_Arduino
 * 
 */

#include <ExoSense.h>

void setup() {
  Serial.begin(9600);
  ExoSense.setup();
  while (!Serial) ;
  Serial.println("Ready");
}

void loop() {
  OPT3001 result = ExoSense.light.readResult();
  if (result.error == NO_ERROR) {
    Serial.print("Luminosity: ");
    Serial.print(result.lux);
    Serial.println(" lux");
  } else {
    Serial.print("Error: ");
    Serial.println(result.error);
  }
  delay(500);
}
