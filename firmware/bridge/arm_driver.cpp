/*
 * arm_driver.cpp
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#include "global.h"
#include "i2c.h"
#include "arm_driver.h"

static const uint8_t ARM_DRIVER_ADDRESS = 0x12;

static uint8_t get(uint8_t motor, uint8_t command) {
	i2c::start();

	i2c::write(i2c::addressToWrite(ARM_DRIVER_ADDRESS));
	i2c::write(command);
	i2c::write(motor);

	i2c::start();
	i2c::write(i2c::addressToRead(ARM_DRIVER_ADDRESS));

	uint8_t val = i2c::read(i2c::NACK);

	i2c::stop();

	return val;
}

static void set(uint8_t motor, uint8_t value, uint8_t command) {
	i2c::start();

	i2c::write(i2c::addressToWrite(ARM_DRIVER_ADDRESS));
	i2c::write(command);
	i2c::write(motor);
	i2c::write(value);

	i2c::stop();
}

static void sendOneByte(uint8_t command) {
	i2c::start();

	i2c::write(i2c::addressToWrite(ARM_DRIVER_ADDRESS));
	i2c::write(command);

	i2c::stop();
}

static uint8_t recvOneByte(uint8_t command) {
	i2c::start();

	i2c::write(i2c::addressToWrite(ARM_DRIVER_ADDRESS));
	i2c::write(command);

	i2c::start();
	i2c::write(i2c::addressToRead(ARM_DRIVER_ADDRESS));

	uint8_t val = i2c::read(i2c::NACK);

	i2c::stop();

	return val;
}

void arm::calibrate() {
	sendOneByte(CHAR_ARM_CALIBRATE);
}

void arm::brake() {
	sendOneByte(CHAR_ARM_BRAKE);
}

bool arm::isCalibrated() {
	if (recvOneByte(CHAR_ARM_IS_CALIBRATED) == CHAR_ARM_TRUE) return true;
	else return false;
}

arm::Mode arm::getMode() {
	return (arm::Mode) recvOneByte(CHAR_ARM_GET_MODE);
}

arm::Direction arm::getDirection(arm::Motor motor) {
	return (arm::Direction) get(motor, CHAR_ARM_GET_DIRECTION);
}

void arm::setDirection(arm::Motor motor, arm::Direction direction) {
	set(motor, direction, CHAR_ARM_SET_DIRECTION);
}

uint8_t arm::getSpeed(arm::Motor motor) {
	return get(motor, CHAR_ARM_GET_SPEED);
}

void arm::setSpeed(arm::Motor motor, uint8_t speed) {
	set(motor, speed, CHAR_ARM_SET_SPEED);
}

uint8_t arm::getPosition(arm::Motor motor) {
	return get(motor, CHAR_ARM_GET_POSITION);
}

void arm::setPosition(arm::Motor motor, uint8_t position) {
	set(motor, position, CHAR_ARM_SET_POSITION);
}
