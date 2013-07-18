/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <inttypes.h>

#define BUTTON_ENTER	BUTTON_S1
#define BUTTON_UP		BUTTON_S3
#define BUTTON_DOWN		BUTTON_S2

#define BUTTON_PORT	D
#define BUTTON_S1	4
#define BUTTON_S2	3
#define BUTTON_S3	2

#define BUTTON_DEBOUNCE_TIME 50 

uint8_t buttonPressed(uint8_t button);

#define buttons_init() { DDR(BUTTON_PORT) &= ~((1<<BUTTON_S1) | (1<<BUTTON_S2) | (1<<BUTTON_S3)); \
				PORT(BUTTON_PORT) |= (1<<BUTTON_S1) | (1<<BUTTON_S2) | (1<<BUTTON_S3); }

#endif
