/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja
#  Michał Słomkowski 2011
#  www.flylab.ovh.org m.slomkowski@gmail.com
*/

// main module

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "pwm.h"
#include "i2c-slave.h"

#include "uart.h"

int main()
{
	motor_init();

#if DEBUG
	uart_init();
	// debug pin set to output
	DDR(DEBUG_PORT) |= (1<<DEBUG_PIN);
#endif
	i2c_initialize((I2C_SLAVE_ADDRESS << 1));

	sei();

	// Watchdog
#if !DEBUG
	wdt_enable(WDTO_120MS);		
#endif

	while(1)
	{
		// generic
		if (i2c_reply_ready())
		{
			if(i2c_wrbuf[0] == CHAR_MOTOR_GET_SPEED)
			{
				i2c_rdbuf[0] = motor_get_speed(i2c_wrbuf[1]);
			}
			else
			{
				i2c_rdbuf[0] = motor_get_direction(i2c_wrbuf[1]);
			}

			i2c_reply_done(1);
		}
	
		if (i2c_message_ready())
		{
			switch(i2c_wrbuf[0])
			{
				case CHAR_MOTOR_BRAKE:
					motor_set_direction(MOTOR1_IDENTIFIER, MOTOR_STOP);
					motor_set_direction(MOTOR2_IDENTIFIER, MOTOR_STOP);
					break;
				case CHAR_MOTOR_SET_SPEED:
					motor_set_speed(i2c_wrbuf[1], i2c_wrbuf[2]);
					break;
				case CHAR_MOTOR_SET_DIRECTION:
					motor_set_direction(i2c_wrbuf[1], i2c_wrbuf[2]);
					break;
				
			};
			i2c_message_done();
		}

#if !DEBUG
		wdt_reset();
#endif
	}
}

