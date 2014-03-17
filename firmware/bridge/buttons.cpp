/*
 * buttons.cpp
 *
 *  Created on: 07-08-2013
 *      Author: michal
 */

#include "global.hpp"
#include <avr/io.h>
#include "delay.hpp"
#include "buttons.hpp"

#define BUTTON_PORT_ENTER	E
#define BUTTON_ENTER		6

#define BUTTON_PORT_UP	B
#define BUTTON_UP		4

#define BUTTON_PORT_DOWN	D
#define BUTTON_DOWN		7

static const uint8_t DEBOUNCE_TIME = 30; // ms

using namespace buttons;

static Buttons buttonsState;

void buttons::init() {
	DDR(BUTTON_PORT_UP) &= ~(1 << BUTTON_UP);
	DDR(BUTTON_PORT_DOWN) &= ~(1 << BUTTON_DOWN);
	DDR(BUTTON_PORT_ENTER) &= ~(1 << BUTTON_ENTER);

	PORT(BUTTON_PORT_UP) |= (1 << BUTTON_UP);
	PORT(BUTTON_PORT_DOWN) |= (1 << BUTTON_DOWN);
	PORT(BUTTON_PORT_ENTER) |= (1 << BUTTON_ENTER);
}

Buttons *buttons::getButtonsState(bool debounce) {

	buttonsState.enter = bit_is_clear(PIN(BUTTON_PORT_ENTER), BUTTON_ENTER);
	buttonsState.up = bit_is_clear(PIN(BUTTON_PORT_UP), BUTTON_UP);
	buttonsState.down = bit_is_clear(PIN(BUTTON_PORT_DOWN), BUTTON_DOWN);

	if (not debounce) {
		return &buttonsState;
	}

	if (not (buttonsState.enter or buttonsState.up or buttonsState.down)) {
		return &buttonsState;
	}

	delay::waitMs(DEBOUNCE_TIME);

	buttonsState.enter &= bit_is_clear(PIN(BUTTON_PORT_ENTER), BUTTON_ENTER);
	buttonsState.up &= bit_is_clear(PIN(BUTTON_PORT_UP), BUTTON_UP);
	buttonsState.down &= bit_is_clear(PIN(BUTTON_PORT_DOWN), BUTTON_DOWN);

	return &buttonsState;
}

