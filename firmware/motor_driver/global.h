/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja
#  moduł bridge - most między RS232 i magistralą główną
#  Michał Słomkowski 2011
#  www.flylab.ovh.org m.slomkowski@gmail.com
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/*
   #######
   ####### Configuration
   #######
*/

#define DEBUG 0

// used only for debug purposes - to show the length of the processing some functions on oscilloscope
// pin 2
#define DEBUG_PORT D
#define DEBUG_PIN 0

// set system clock to 4 MHz
#define F_CPU 4000000UL

#include <inttypes.h>

/*
   #######
   ####### Useful macros
   #######
*/

#define PORT(x) XPORT(x)
#define XPORT(x) (PORT##x)
// *** Pin
#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)
// *** DDR
#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)

#define debug_up() { PORT(DEBUG_PORT) |= (1 << DEBUG_PIN); }
#define debug_down() { PORT(DEBUG_PORT) &= ~(1 << DEBUG_PIN); }

#define TRUE 1
#define FALSE 0

#include "motor_driver-commands.h"

#endif

