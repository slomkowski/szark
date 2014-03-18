/*
 * led.cpp
 *
 *  Project: bridge
 *  Created on: 18 mar 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include <avr/io.h>

#include "global.hpp"
#include "led.hpp"

using namespace led;

#define LED_GREEN_PORT		D
#define LED_GREEN			5
#define LED_YELLOW_PORT	B
#define LED_YELLOW			0

void led::init() {
	PORT(LED_GREEN_PORT) |= (1 << LED_GREEN);
	PORT(LED_YELLOW_PORT) |= (1 << LED_YELLOW);

	DDR(LED_GREEN_PORT) |= (1 << LED_GREEN);
	DDR(LED_YELLOW_PORT) |= (1 << LED_YELLOW);
}

void led::setState(Diode diode, bool enabled) {
	if (diode == YELLOW) {
		PORT(LED_YELLOW_PORT) =
			enabled ? PORT(LED_YELLOW_PORT) & ~(1 << LED_YELLOW) : PORT(LED_YELLOW_PORT) | (1 << LED_YELLOW);
	} else {
		PORT(LED_GREEN_PORT) =
			enabled ? PORT(LED_GREEN_PORT) & ~(1 << LED_GREEN) : PORT(LED_GREEN_PORT) | (1 << LED_GREEN);
	}
}

void led::setState(bool enabled) {
	if (enabled) {
		PORT(LED_GREEN_PORT) &= ~(1 << LED_GREEN);
		PORT(LED_YELLOW_PORT) &= ~(1 << LED_YELLOW);
	} else {
		PORT(LED_GREEN_PORT) |= (1 << LED_GREEN);
		PORT(LED_YELLOW_PORT) |= (1 << LED_YELLOW);
	}
}

void led::toggleState(Diode diode) {
	if (diode == YELLOW) {
		PORT(LED_YELLOW_PORT) ^= (1 << LED_YELLOW);
	} else {
		PORT(LED_GREEN_PORT) ^= (1 << LED_GREEN);
	}
}
