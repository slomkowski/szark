/*
 * motor_driver.h
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#ifndef MOTOR_DRIVER_H_
#define MOTOR_DRIVER_H_

#include "motor_driver-commands.hpp"
#include <stdint.h>

namespace motor {
	const motor::Motor LEFT = motor::MOTOR1;
	const motor::Motor RIGHT = motor::MOTOR2;

	motor::Direction getDirection(motor::Motor motor);
	void setDirection(motor::Motor motor, motor::Direction direction);

	uint8_t getSpeed(motor::Motor motor);
	void setSpeed(motor::Motor motor, uint8_t speed);

	void brake();
}

#endif /* MOTOR_DRIVER_H_ */
