/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _ARM_H_
#define _ARM_H_

#include "global.h"

#define ARM_DRIVER_ADDRESS 0x30

#include "arm_driver-commands.h"

#define CHAR_ARM_GET_ALL_POSITIONS 'P'
#define CHAR_ARM_GET_ALL_DIRECTIONS 'D'

#include <inttypes.h>

uint8_t arm_get(uint8_t arm, uint8_t command);
void arm_set(uint8_t arm, uint8_t value, uint8_t command);

void arm_one_byte(uint8_t byte);
uint8_t arm_get_one_byte(uint8_t command);

bool arm_is_calibrated();

#define arm_get_calibration_status() arm_get_one_byte(CHAR_ARM_IS_CALIBRATED)
#define arm_get_mode() arm_get_one_byte(CHAR_ARM_GET_MODE)

#define arm_calibrate()	arm_one_byte(CHAR_ARM_CALLIBRATE)
#define arm_brake()		arm_one_byte(CHAR_ARM_BRAKE)

#define arm_get_direction(arm)	arm_get(arm, CHAR_ARM_GET_DIRECTION)
#define arm_get_speed(arm)		arm_get(arm, CHAR_ARM_GET_SPEED)
#define arm_get_position(arm)		arm_get(arm, CHAR_ARM_GET_POSITION)

#define arm_set_speed(arm, speed)			arm_set(arm, speed, CHAR_ARM_SET_SPEED)
#define arm_set_direction(arm, direction)	arm_set(arm, direction, CHAR_ARM_SET_DIRECTION)
#define arm_set_position(arm, position)		arm_set(arm, position, CHAR_ARM_SET_POSITION)

#endif
