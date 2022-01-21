/*
  ExoSense.h - Exo Sense RP base library

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef ExoSense_h
#define ExoSense_h

#include "libs/ClosedCube_OPT3001_Arduino/src/ClosedCube_OPT3001.h"

#define EXOS_PIN_DI1 28
#define EXOS_PIN_DI2 29
#define EXOS_PIN_TTL1 26
#define EXOS_PIN_TTL2 27
#define EXOS_PIN_DO1 10
#define EXOS_PIN_LED 14
#define EXOS_PIN_BUZZER 13
#define EXOS_PIN_BUZZER_PWM 12
#define EXOS_PIN_PIR 23
#define EXOS_PIN_RS485_TX 16
#define EXOS_PIN_RS485_RX 17
#define EXOS_PIN_RS485_TXEN_N 15
#define EXOS_PIN_I2S_SCK 6
#define EXOS_PIN_I2S_WS 7
#define EXOS_PIN_I2S_SD 8
#define EXOS_PIN_I2C_SDA 0
#define EXOS_PIN_I2C_SCL 1

#define EXOS_I2C_ADDR_SENS_TEMP_RH 0x44
#define EXOS_I2C_ADDR_SENS_LIGHT 0x45
#define EXOS_I2C_ADDR_SENS_SYS_TEMP_U9 0x48
#define EXOS_I2C_ADDR_SENS_SYS_TEMP_U16 0x49
#define EXOS_I2C_ADDR_SENS_VIBR 0x55
#define EXOS_I2C_ADDR_SENS_VOC 0x59
#define EXOS_I2C_ADDR_SECELEM 0x60
#define EXOS_I2C_ADDR_RTC 0x6f
#define EXOS_I2C_ADDR_RTC_EEPROM 0x57
#define EXOS_I2C_ADDR_EERAM_SRAM 0x50
#define EXOS_I2C_ADDR_EERAM_CTRL 0x18

#define SERIAL_PORT_MONITOR Serial
#define SERIAL_PORT_HARDWARE Serial1

#define EXOS_RS485 SERIAL_PORT_HARDWARE

class ExoSenseClass
{
  public:
    ClosedCube_OPT3001 light;

    ExoSenseClass();
    void setup();
    void rs485TxEn(bool enabled);

  private:

};

extern ExoSenseClass ExoSense;

#endif
