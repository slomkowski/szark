/*
 * i2c.cpp
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "i2c.h"

using namespace i2c;

/* tricky timer
 i2c functions have timer attached, so if the device doesn't respond, function waits a fixed period of time and then returns.
 If there was no timer, function would loop forever
 */
#define TIMER_ENABLE true

static volatile bool timerNotClear = true;

static ErrorCode status = OK;

void i2c::init() {
	TWBR = (((F_CPU / 1000l) / CLOCK_FREQUENCY) - 16) / 2;
	TWCR = 1 << TWEN | 1 << TWEA | 1 << TWINT;

#if TIMER_ENABLE
	// timer
	TIMSK0 |= (1 << TOIE0);
#endif
}

static void setUpTimer() {
#if TIMER_ENABLE
	TCNT0 = 0;
	timerNotClear = true;
	TCCR0B = (1 << CS02);// | (1 << CS00);// clkio/64
#endif
}

static void waitForOperationComplete() {
	while (!(TWCR & (1 << TWINT)) && timerNotClear) {
	}

#if TIMER_ENABLE
	if (timerNotClear) {
		status = OK;
	} else {
		status = TIMEOUT;
	}
#endif
}

#if TIMER_ENABLE
ISR(TIMER0_OVF_vect, ISR_NOBLOCK) {
	timerNotClear = false;
	TCCR0B = 0; // stop timer
}
#endif

void i2c::start() {
	setUpTimer();

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	waitForOperationComplete();
}

// I2C Stop
void i2c::stop() {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	while (!(TWCR & (1 << TWSTO)) && timerNotClear) {
	}
}

void i2c::write(uint8_t byte) {
	setUpTimer();

	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	waitForOperationComplete();
}

uint8_t i2c::read(Acknowledgement ack) {
	setUpTimer();

	if (ack == ACK) {
		TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
	} else {
		TWCR = ((1 << TWINT) | (1 << TWEN));
	}

	waitForOperationComplete();

	return TWDR ;
}

ErrorCode i2c::getLastCommandStatus() {
	return status;
}
