/*
 * analog.cpp
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#include "global.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "analog.hpp"

#define ANALOG_VOLTAGE_CHANNEL 0x4
#define ANALOG_CURRENT_CHANNEL 0x5

static volatile uint16_t rawVoltage;
static volatile uint16_t rawCurrent;

void analog::init() {
	DDRC &= ~((1 << ANALOG_VOLTAGE_CHANNEL) | (1 << ANALOG_CURRENT_CHANNEL)); // set as inputs
	PORTC &= ~((1 << ANALOG_VOLTAGE_CHANNEL) | (1 << ANALOG_CURRENT_CHANNEL)); // disable pull-up

	// Internal 1.1 V Voltage Reference with external capacitor at AREF pin
	ADMUX = (1 << REFS0) | (1 << REFS1);

	DIDR0 = (1 << ANALOG_VOLTAGE_CHANNEL) | (1 << ANALOG_CURRENT_CHANNEL);

	// enable ADC, start conversion, set prescaler to 64 - ADC freq: ~ 125kHz
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);
}

ISR(ADC_vect, ISR_NOBLOCK) {
	static bool channelChooser = false;

	if (channelChooser) {
		ADMUX = (ADMUX & 0xf0) | ANALOG_CURRENT_CHANNEL;
		rawVoltage = ADCW;
		channelChooser = false;
	} else {
		ADMUX = (ADMUX & 0xf0) | ANALOG_VOLTAGE_CHANNEL;
		rawCurrent = ADCW;
		channelChooser = true;
	}

	ADCSRA |= (1 << ADSC);
}

uint16_t analog::getRawVoltage() {
	return rawVoltage;
}

uint16_t analog::getRawCurrent() {
	return rawCurrent;
}

