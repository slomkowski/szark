/*
 * killswitch.cpp
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#include "global.h"
#include <avr/io.h>
#include <util/delay.h>
#include "killswitch.h"

#define KS_PORT C
#define KS_PIN 3

void killswitch::init() {
	DDR(KS_PORT) |= (1 << KS_PIN);
	setActive(true);
}

void killswitch::setActive(bool active) {
	if (active == true) {
		PORT(KS_PORT) &= ~(1 << KS_PIN);
	} else if (isActive() == true) {
		PORT(KS_PORT) |= (1 << KS_PIN);
		_delay_ms(50);
	}
}

bool killswitch::isActive() {
	if (bit_is_clear(PIN(KS_PORT), KS_PIN)) {
		return true;
	}
	return false;
}
