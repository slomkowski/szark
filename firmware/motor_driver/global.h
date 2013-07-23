#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/*
 * Configuration
 */

#define DEBUG true

// set system clock to 4 MHz
#define F_CPU 4000000UL

#include <inttypes.h>

/*
 * Useful macros
 */

#define PORT(x) XPORT(x)
#define XPORT(x) (PORT##x)
// *** Pin
#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)
// *** DDR
#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)

/*
 * Debug pin - it's NOT Atmel's DebugWIRE feature
 * used only for debug purposes - to show the length of some functions's processing time on an oscilloscope
 */
// pin 2
#define DEBUG_PORT D
#define DEBUG_PIN 0

#include <avr/io.h>

static inline void debugUp() {
	PORT(DEBUG_PORT) |= (1 << DEBUG_PIN);
}
static inline void debugDown() {
	PORT(DEBUG_PORT) &= ~(1 << DEBUG_PIN);
}

#include "motor_driver-commands.h"

#endif

