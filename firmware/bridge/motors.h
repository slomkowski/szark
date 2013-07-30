/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _MOTORS_H_
#define _MOTORS_H_

#include "global.h"

#define MOTOR_DRIVER_ADDRESS 0x12

#include "motor_driver-commands.h"

#define MOTOR_LEFT CHAR_MOTOR1
#define MOTOR_RIGHT CHAR_MOTOR2

#include <inttypes.h>

uint8_t motor_get(uint8_t command, uint8_t motor);
void motor_set(uint8_t command, uint8_t motor, uint8_t value);

void brake();

#define motor_get_direction(motor) motor_get(CHAR_MOTOR_GET_DIRECTION, motor)
#define motor_get_speed(motor) motor_get(CHAR_MOTOR_GET_SPEED, motor)

#define motor_set_speed(motor, speed) motor_set(CHAR_MOTOR_SET_SPEED, motor, speed)
#define motor_set_direction(motor, direction) motor_set(CHAR_MOTOR_SET_DIRECTION, motor, direction)

#endif
