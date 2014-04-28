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
#include "lcd.hpp"

#define KS_PORT D
#define KS_PIN 4

static volatile bool causedByHardware = false;
static volatile bool causedBySoftware = true;

void killswitch::init() {
	setActive(true);
}

void killswitch::setActive(bool active) {
	if (active == true) {
		if (not isActive()) {
			causedBySoftware = true;
			causedByHardware = false;
		}

		DDR(KS_PORT) |= (1 << KS_PIN);
		PORT(KS_PORT) &= ~(1 << KS_PIN);
	} else if (isActive() == true) { // if going from active to inactive
		causedByHardware = false;
		causedBySoftware = false;

		DDR(KS_PORT) &= ~(1 << KS_PIN);
		PORT(KS_PORT) |= (1 << KS_PIN);

		if (isActive() == true) { // if hardware keeps it down
			causedByHardware = true;
		}

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

bool killswitch::isCausedBySoftware() {
	return causedBySoftware;
}

void killswitch::checkHardware() {
	if (killswitch::isActive()) {
		if (not causedBySoftware) {
			causedByHardware = true;
		}

		killswitch::setActive(true);
	} else {
		causedByHardware = false;
		causedBySoftware = false;
	}
}
