/*
 * buttons.cpp
 *
 *  Created on: 07-08-2013
 *      Author: michal
 */

#include "global.h"
#include <avr/io.h>
#include <util/delay.h>
#include "buttons.h"

#define BUTTON_PORT	D
#define BUTTON_ENTER	1
#define BUTTON_UP		0
#define BUTTON_DOWN	2

static const uint8_t DEBOUNCE_TIME = 50; // ms

using namespace buttons;

static Buttons buttonsState;

void buttons::init() {
	DDR(BUTTON_PORT) &= ~((1 << BUTTON_UP) | (1 << BUTTON_DOWN) | (1 << BUTTON_ENTER));
	PORT(BUTTON_PORT) |= (1 << BUTTON_UP) | (1 << BUTTON_DOWN) | (1 << BUTTON_ENTER);
}

Buttons *buttons::getButtonsState() {

	buttonsState.enter = bit_is_clear(PIN(BUTTON_PORT), BUTTON_ENTER);
	buttonsState.up = bit_is_clear(PIN(BUTTON_PORT), BUTTON_UP);
	buttonsState.down = bit_is_clear(PIN(BUTTON_PORT), BUTTON_DOWN);

	if (not (buttonsState.enter or buttonsState.up or buttonsState.down)) {
		return &buttonsState;
	}

	_delay_ms(DEBOUNCE_TIME);

	buttonsState.enter &= bit_is_clear(PIN(BUTTON_PORT), BUTTON_ENTER);
	buttonsState.up &= bit_is_clear(PIN(BUTTON_PORT), BUTTON_UP);
	buttonsState.down &= bit_is_clear(PIN(BUTTON_PORT), BUTTON_DOWN);

	return &buttonsState;
}

