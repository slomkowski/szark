#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "controller.h"
#include "pwm.h"

static uint8_t limit_pwm_val(int16_t value) {
	if (value < 0) {
		return 0;
	} else if (value > 0xff) {
		return 0xff;
	} else {
		return value;
	}
}

uint8_t controller(uint16_t actual, uint8_t reference, int16_t *prevError) {
	int16_t error = actual - reference;
}

ISR(TIMER1_COMPA_vect) {
	int16_t err1, err2, out1, out2;
	static int16_t prevErr1, prevErr2;

	if (motor1_direction != MOTOR_STOP) {
		err1 = motor1_counter - motor1_ref;

		out1 = PWM1_REG - ((CONTROLLER_P_REG * err1 / 100) + (CONTROLLER_D_REG * (err1 - prevErr1) / 100));

		prevErr1 = err1;

		PWM1_REG = limit_pwm_val(out1);
	} else {
		prevErr1 = 0;
	}

	if (motor2_direction != MOTOR_STOP) {
		err2 = motor2_counter - motor2_ref;

		out2 = PWM2_REG - ((CONTROLLER_P_REG * err2 / 100) + (CONTROLLER_D_REG * (err2 - prevErr2) / 100));

		prevErr2 = err2;

		PWM2_REG = limit_pwm_val(out2);
	} else {
		prevErr2 = 0;
	}

	// clear the counters
	motor1_counter = 0;
	motor2_counter = 0;
}

