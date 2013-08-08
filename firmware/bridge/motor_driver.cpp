/*
 * motor_driver.cpp
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#include "global.h"
#include "motor_driver.h"
#include "i2c.h"

static const uint8_t MOTOR_DRIVER_ADDRESS = 0x12;

using namespace motor;

static uint8_t get(uint8_t command, uint8_t motor) {
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

static void set(uint8_t command, uint8_t motor, uint8_t value) {
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

motor::Direction motor_driver::getDirection(motor::Motor motor) {
	return (Direction) get(CHAR_MOTOR_GET_DIRECTION, motor);
}

void motor_driver::setDirection(motor::Motor motor, motor::Direction direction) {
	set(CHAR_MOTOR_SET_DIRECTION, motor, (uint8_t) direction);
}

uint8_t motor_driver::getSpeed(motor::Motor motor) {
	return get(CHAR_MOTOR_GET_SPEED, motor);
}

void motor_driver::setSpeed(motor::Motor motor, uint8_t speed) {
	set(CHAR_MOTOR_SET_SPEED, motor, speed);
}
