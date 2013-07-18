/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Micha¿ S¿omkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#include "global.h"

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "analog.h"

#define FREEMODE 1

#if FREEMODE == 0

void analog_init()
{
	DDRC &= ~((1 << ANALOG_VOLTAGE_PIN) | (1 << ANALOG_CURRENT_PIN)); // set as inputs
	PORTC &= ~((1 << ANALOG_VOLTAGE_PIN) | (1 << ANALOG_CURRENT_PIN)); // disable pullup

	// Internal 2.56V Voltage Reference with external capacitor at AREF pin
	ADMUX = (1<<REFS0) | (1<<REFS1);

	// enable ADC, start convertion, set prescaler to 64 - ADC freq: ~ 125kHz
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1);
}

uint16_t analog_get_voltage()
{
	ADMUX = (ADMUX & 0xf0) | ANALOG_VOLTAGE_CHANNEL;

	ADCSRA |= (1<<ADSC); // start measuring

	while(!(ADCSRA & (1<<ADIF)));

	return ADCW;
}

uint16_t analog_get_current()
{
	ADMUX = (ADMUX & 0xf0) | ANALOG_CURRENT_CHANNEL;

	ADCSRA |= (1<<ADSC); // start measuring

	while(!(ADCSRA & (1<<ADIF)));

	return ADCW;
}

#else

volatile uint16_t voltage, current;
static volatile uint8_t chooser;

void analog_init()
{
	DDRC &= ~((1 << ANALOG_VOLTAGE_PIN) | (1 << ANALOG_CURRENT_PIN)); // set as inputs
	PORTC &= ~((1 << ANALOG_VOLTAGE_PIN) | (1 << ANALOG_CURRENT_PIN)); // disable pullup

	// Internal 2.56V Voltage Reference with external capacitor at AREF pin
	ADMUX = (1<<REFS0) | (1<<REFS1);

	// enable ADC, start convertion, set prescaler to 64 - ADC freq: ~ 125kHz
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADIE);
}

ISR(ADC_vect)
{
	if(chooser)
	{
		ADMUX = (ADMUX & 0xf0) | ANALOG_CURRENT_CHANNEL;
		voltage = ADCW;
		chooser = 0;
	}
	else
	{
		ADMUX = (ADMUX & 0xf0) | ANALOG_VOLTAGE_CHANNEL;
		current = ADCW;
		chooser = 1;
	}

	ADCSRA |= (1<<ADSC); // start convertion
}

/*uint16_t analog_get_voltage()
{
	return voltage;
}

uint16_t analog_get_current()
{
	return current;
}*/

#endif
