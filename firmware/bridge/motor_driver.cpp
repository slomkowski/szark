/*
 * motor_driver.cpp
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#include "global.h"
#include "motor_driver.h"
#include "i2c.h"

const uint8_t MOTOR_DRIVER_ADDRESS = 0x12;

uint8_t motor_driver::get(uint8_t command, uint8_t motor) {
	uint8_t val;

	i2c::start();

	i2c::write(i2c::addressToWrite(MOTOR_DRIVER_ADDRESS));
	i2c::write(command);
	i2c::write(motor);

	i2c::start();
	i2c::write(i2c::addressToRead(MOTOR_DRIVER_ADDRESS));

	val = i2c::read(i2c::NACK);

	i2c::stop();

	return val;
}

void motor_driver::set(uint8_t command, uint8_t motor, uint8_t value) {
	i2c::start();

	i2c::write(i2c::addressToWrite(MOTOR_DRIVER_ADDRESS));
	i2c::write(command);
	i2c::write(motor);
	i2c::write(value);

	i2c::stop();
}

void motor_driver::brake() {
	i2c::start();

	i2c::write(i2c::addressToWrite(MOTOR_DRIVER_ADDRESS));
	i2c::write(CHAR_MOTOR_BRAKE);

	i2c::stop();
}

