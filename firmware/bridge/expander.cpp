/*
 * expander.cpp
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#include "i2c.hpp"
#include "expander.hpp"

static const uint8_t EXPANDER_ADDRESS = 0x64;

static const uint8_t WRITE_AND_TRANSFER_DATA = 0x44;
//static const uint8_t WRITE_DATA = 0x11;
//static const uint8_t TRANSFER_DATA = 0x22;
static const uint8_t READ_DATA = 0x11;

void expander::setValue(uint8_t data) {
	i2c::start();

	i2c::write(i2c::addressToWrite(EXPANDER_ADDRESS));
	i2c::write(WRITE_AND_TRANSFER_DATA);
	i2c::write(data);

	i2c::stop();
}

uint8_t expander::getValue() {

	i2c::start();

	i2c::write(i2c::addressToWrite(EXPANDER_ADDRESS));
	i2c::write(READ_DATA);

	i2c::start();
	i2c::write(i2c::addressToRead(EXPANDER_ADDRESS));
	uint8_t val = i2c::read(i2c::NACK);

	i2c::stop();

	return val;
}
