/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

/* support for hardware TWI (I2C) interface */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "global.h"
#include "i2c.h"

#define I2C_CALC_TWBR ((((F_CPU/1000l)/I2C_CLOCK)-16)/2)

#define TIMER_ENABLE 1

void i2c_init()
{
	TWBR = I2C_CALC_TWBR;
	TWCR = 1<<TWEN | 1<<TWEA | 1<<TWINT;

#if TIMER_ENABLE
	// timer
	TIMSK |= (1<<TOIE0);
#endif
}

/* tricky timer
   i2c functions have timer attached, so if the device doesn't respond, function waits a fixed period of time and then returns.
   If there was no timer, function would loop forever
   */
static volatile uint8_t timerNotClear = 1;

static void set_timer()
{
#if TIMER_ENABLE
	// setup timer
	TCNT0 = 150;
	timerNotClear = 1;
	TCCR0 = (1<<CS01) | (1<<CS00); // clkio/64
#endif
}

#if TIMER_ENABLE
ISR(TIMER0_OVF_vect)
{
	timerNotClear = 0;
	TCCR0 = 0; // stop timer
}
#endif

// I2C Start
// Returns 1 if OK
void i2c_start()
{
	set_timer();

	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	//while(!(TWCR & (1<<TWINT)));
	while(!(TWCR & (1<<TWINT)) && timerNotClear);
}

// I2C Stop
void i2c_stop()
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	//while ((TWCR & (1<<TWSTO)));
	while(!(TWCR & (1<<TWSTO)) && timerNotClear);
}

void i2c_write(uint8_t byte)
{
	set_timer();

	TWDR = byte;
	TWCR = (1<<TWINT) | (1<<TWEN);
	//while (!(TWCR & (1<<TWINT)));
	while(!(TWCR & (1<<TWINT)) && timerNotClear);
}

uint8_t i2c_read(uint8_t ack)
{
	set_timer();

	TWCR = ack ? ((1 << TWINT) | (1 << TWEN) | (1 << TWEA)) : ((1 << TWINT) | (1 << TWEN));

	while(!(TWCR & (1<<TWINT)) && timerNotClear);

	return TWDR;
}


