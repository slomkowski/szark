/*
 * killswitch.cpp
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#include "global.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "delay.h"
#include "killswitch.h"

#define KS_PORT C
#define KS_PIN 3

static bool causedByHardware = false;

void killswitch::init() {
	setActive(true);

	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT11);
}

void killswitch::setActive(bool active) {
	if (active == true) {
		DDR(KS_PORT) |= (1 << KS_PIN);
		PORT(KS_PORT) &= ~(1 << KS_PIN);
	} else if (isActive() == true) {
		causedByHardware = false;

		DDR(KS_PORT) &= ~(1 << KS_PIN);
		PORT(KS_PORT) |= (1 << KS_PIN);

		delay::waitMs(150);
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

ISR(PCINT1_vect, ISR_NOBLOCK) {
	causedByHardware = true;
	if (killswitch::isActive()) {
		killswitch::setActive(true);
	}
}
