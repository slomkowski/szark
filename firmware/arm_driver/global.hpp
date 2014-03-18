#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/*
   #######
   ####### Configuration
   #######
*/

#define I2C_SLAVE_ADDRESS 0x30
#define I2C_BUFFER_SIZE 5      // Reserves memory for the drivers transceiver buffer.

#define DEBUG 1

#define WATCHDOG_ENABLE 1

#define F_CPU 8000000UL

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

#endif

