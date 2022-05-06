/*
 * ExoSenseInternalTemp.ino -
 *   Using Exo Sense RP's internal temperature sensors
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
 * The ExoSense.lm75a_u9 and ExoSense.lm75a_u16 objects are
 * instaces of the M2M_LM75A class from the M2M Solutions's M2M_LM75A
 * library, included in the ExoSense library.
 *
 * For other examples:
 * https://github.com/sfera-labs/arduino-M2M_LM75A
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
  Serial.print("Temperature U9: ");
  Serial.print(ExoSense.lm75a_u9.getTemperature());
  Serial.println(" C");

  Serial.print("Temperature U16: ");
  Serial.print(ExoSense.lm75a_u16.getTemperature());
  Serial.println(" C");

  delay(1000);
}
