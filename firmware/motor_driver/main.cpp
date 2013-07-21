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

int main() {
#if DEBUG
	// debug pin set to output
	DDR(DEBUG_PORT) |= (1<<DEBUG_PIN);
#endif

	motor::init();
	i2c_initialize((I2C_SLAVE_ADDRESS << 1));

	sei();

	// Watchdog
#if !DEBUG
	wdt_enable(WDTO_120MS);
#endif

	while (1) {
		// generic
		if (i2c_reply_ready()) {
			if (i2c_wrbuf[0] == CHAR_MOTOR_GET_SPEED) {
				i2c_rdbuf[0] = motor::getSpeed(i2c_wrbuf[1]);
			} else {
				i2c_rdbuf[0] = motor::getDirection(i2c_wrbuf[1]);
			}

			i2c_reply_done(1);
		}

		if (i2c_message_ready()) {
			switch (i2c_wrbuf[0]) {
			case CHAR_MOTOR_BRAKE:
				motor::setDirection(MOTOR1_IDENTIFIER, MOTOR_STOP);
				motor::setDirection(MOTOR2_IDENTIFIER, MOTOR_STOP);
				break;
			case CHAR_MOTOR_SET_SPEED:
				motor::setSpeed(i2c_wrbuf[1], i2c_wrbuf[2]);
				break;
			case CHAR_MOTOR_SET_DIRECTION:
				motor::setDirection(i2c_wrbuf[1], i2c_wrbuf[2]);
				break;

			};
			i2c_message_done();
		}

#if !DEBUG
		wdt_reset();
#endif
	}
}

