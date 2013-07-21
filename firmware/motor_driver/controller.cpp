#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "controller.h"
#include "pwm.h"

using namespace motor;

static uint8_t limit_pwm_val(int16_t value) {
	if (value < 0) {
		return 0;
	} else if (value > 0xff) {
		return 0xff;
	} else {
		return value;
	}
}

ISR(TIMER1_COMPA_vect) {
	int16_t output, error;
	static int16_t prevErr[2];

	if (motor1.direction != STOP) {
		error = motor1.counter - motor1.refSpeed;

		output = PWM1_REG - ((CONTROLLER_P_REG * error / 100) /*+ (CONTROLLER_D_REG * (error - prevErr1) / 100)*/);

		prevErr[0] = error;

		PWM1_REG = limit_pwm_val(output);

	} else {
		prevErr[0] = 0;
	}

	if (motor2.direction != STOP) {
		error = motor2.counter - motor2.refSpeed;

		output = PWM2_REG - ((CONTROLLER_P_REG * error / 100) /*+ (CONTROLLER_D_REG * (error - prevErr2) / 100)*/);

		prevErr[1] = error;

		PWM2_REG = limit_pwm_val(output);
	} else {
		prevErr[1] = 0;
	}

	// clear the counters
	motor1.counter = 0;
	motor2.counter = 0;
}

