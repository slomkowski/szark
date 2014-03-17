/*
 * killswitch.cpp
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#include "global.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "delay.hpp"
#include "killswitch.hpp"

#define KS_PORT D
#define KS_PIN 4

static bool causedByHardware = false;

void killswitch::init() {
	setActive(true);
}

void killswitch::setActive(bool active) {
	if (active == true) {
		DDR(KS_PORT) |= (1 << KS_PIN);
		PORT(KS_PORT) &= ~(1 << KS_PIN);
	} else if (isActive() == true) {
		causedByHardware = false;

		DDR(KS_PORT) &= ~(1 << KS_PIN);
		PORT(KS_PORT) |= (1 << KS_PIN);

		delay::waitMs(5);
	}
}

bool killswitch::isActive() {
	if (bit_is_clear(PIN(KS_PORT), KS_PIN)) {
		return true;
	}
	return false;
}

bool killswitch::isCausedByHardware() {
	return causedByHardware;
}

/* TODO zastąpić przerwanie jakimś innym rozwiązaniem
ISR(PCINT1_vect, ISR_NOBLOCK) {
	causedByHardware = true;
	if (killswitch::isActive()) {
		killswitch::setActive(true);
	}
}*/
