/*
 * motor_driver.h
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#ifndef MOTOR_DRIVER_H_
#define MOTOR_DRIVER_H_

#include "motor_driver-commands.h"
#include <stdint.h>

namespace motor_driver {
	uint8_t get(uint8_t command, uint8_t motor);
	void set(uint8_t command, uint8_t motor, uint8_t value);
	void brake();
}

#endif /* MOTOR_DRIVER_H_ */
