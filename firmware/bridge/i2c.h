/*
 * i2c.h
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

namespace i2c {

	// in kHz
	const uint8_t CLOCK_FREQUENCY = 50;

	enum Acknowledgement
		: uint8_t {
			NACK = 0, ACK = 1
	};

	constexpr uint8_t addressToWrite(uint8_t address) {
		return (address << 1);
	}

	constexpr uint8_t addressToRead(uint8_t address) {
		return (address << 1) | 0x1;
	}

	void init();

	void start();
	void stop();

	void write(uint8_t byte);
	uint8_t read(Acknowledgement ack);
}

#endif /* I2C_H_ */
