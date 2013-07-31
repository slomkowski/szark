/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/*
   #######
   ####### Configuration
   #######
*/

#define F_CPU 16000000UL // 16Mhz

#define WATCHDOG_ENABLE 1

/* Respond when set command is sent */
#define SET_COMMAND_ACK 0

/* definition on the characters used in protocol RS232 */

// received
#define RS_INITIALIZATION 'I'
#define RS_STOP_SERVER 'S'

#define RS_MOTOR 'M'
#define RS_LCD_WRITE 'W'
#define RS_BATTERY 'B'
#define RS_EXPANDER 'L'
#define RS_EXPANDER_GET 'g'
#define RS_EXPANDER_SET 's'

#define RS_ARM 'A'

#define RS_ALL_DATA 'Q'

// sent
#define RS_STOP_BUS 'E'

/*#define RS_BUTTON_ESC 'X'
#define RS_BUTTON_NEXT 'Y'
#define RS_BUTTON_ENTER 'Z'*/

#include <inttypes.h>
#include <stdbool.h>

// a global variable used to determine if the device is initialized
// the device keeps low state at the EMERGENCY RESET until it's initialized
// to prevent the motor drivers from running
extern volatile uint8_t enabled;

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

#define EMERGENCY_STOP_PORT	C
#define EMERGENCY_STOP		3

void emergencyStopPerform();
void emergencyStopDisable();
uint8_t emergencyIsStopped();


#endif

