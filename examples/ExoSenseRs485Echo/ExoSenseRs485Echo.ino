/*
 * ExoSenseRs485Echo.ino - Using Exo Sense RP's RS-485 interface
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
 * With this sketch Exo Sense reads whatever is sent on the RS-485
 * port and echoes it back.
 *
 */

#include <ExoSense.h>

#define MAX_LEN 512

byte rxBuff[MAX_LEN + 1];
int rxIdx;

void setup() {
  // Must be called before initializing the serial port
  ExoSense.setup();

  /**
   * Initialize port
   * baud rate: 19200
   * data bits: 8
   * parity: none
   * stop bits: 1
   */
  EXOS_RS485.begin(19200, SERIAL_8N1);
}

void loop() {
  if (EXOS_RS485.available() > 0) {
    rxIdx = 0;

    // Read into buffer while data is available
    while(EXOS_RS485.available() > 0 && rxIdx <= MAX_LEN) {
      rxBuff[rxIdx++] = EXOS_RS485.read();
      if (EXOS_RS485.available() == 0) {
        // give it some extra time to check if
        // any other data is on its way...
        delay(50);
      }
    }

    ExoSense.rs485TxEn(true);

    EXOS_RS485.write(rxBuff, rxIdx);
    EXOS_RS485.flush();

    ExoSense.rs485TxEn(false);
  }

  delay(50);
}
