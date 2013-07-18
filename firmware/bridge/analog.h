/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Micha¿ S¿omkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _ANALOG_H_
#define _ANALOG_H_

#define ANALOG_VOLTAGE_CHANNEL 0x1
#define ANALOG_CURRENT_CHANNEL 0x0

#define ANALOG_VOLTAGE_PIN 1
#define ANALOG_CURRENT_PIN 0

#include <inttypes.h>

//uint16_t analog_get_voltage();
//uint16_t analog_get_current();

extern volatile uint16_t voltage, current;

void analog_init();

#endif
