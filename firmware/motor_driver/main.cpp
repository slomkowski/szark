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

#include "motor.h"
#include "i2c-slave.h"

int main() {
#if DEBUG
	// debug pin set to output
	DDR(DEBUG_PORT) |= (1 << DEBUG_PIN);
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
			motor::Motor motor = (motor::Motor) i2c_wrbuf[1];

			if (i2c_wrbuf[0] == CHAR_MOTOR_GET_SPEED) {
				i2c_rdbuf[0] = motor::getSpeed(motor);
			} else {
				i2c_rdbuf[0] = motor::getDirection(motor);
			}

			i2c_reply_done(1);
		}

		if (i2c_message_ready()) {
			motor::Motor motor = (motor::Motor) i2c_wrbuf[1];

			switch (i2c_wrbuf[0]) {
			case CHAR_MOTOR_BRAKE:
				motor::setDirection(motor::MOTOR1, motor::STOP);
				motor::setDirection(motor::MOTOR2, motor::STOP);
				break;
			case CHAR_MOTOR_SET_SPEED:
				motor::setSpeed(motor, i2c_wrbuf[2]);
				break;
			case CHAR_MOTOR_SET_DIRECTION:
				motor::setDirection(motor, (motor::Direction) i2c_wrbuf[2]);
				break;

			};
			i2c_message_done();
		}

#if !DEBUG
		wdt_reset();
#endif
	}
}

