/*
 * arm_driver.h
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#ifndef ARM_DRIVER_H_
#define ARM_DRIVER_H_

#include <stdint.h>
#include "arm_driver-commands.h"

namespace arm_driver {
	void calibrate();
	void brake();
	bool isCalibrated();

	arm::Mode getMode();

	arm::Direction getDirection(arm::Motor motor);
	void setDirection(arm::Motor, arm::Direction direction);

	uint8_t getSpeed(arm::Motor motor);
	void setSpeed(arm::Motor motor, uint8_t speed);

	uint8_t getPosition(arm::Motor motor);
	void setPosition(arm::Motor motor, uint8_t position);
}

#endif /* ARM_DRIVER_H_ */
