#include "global.h"

#include <inttypes.h>
#include "motor.h"
#include "controller.h"

using namespace motor;

static uint8_t limit8bit(int16_t value) {
	if (value < 0) {
		return 0;
	} else if (value > 0xff) {
		return 0xff;
	} else {
		return value;
	}
}

uint8_t motor::regulate(MotorStruct *motor, uint8_t actualPwmReg) {
	int16_t output, error;
	if (motor->direction != STOP) {
		error = motor->counter - motor->refSpeed;

		output = actualPwmReg - ((CONTROLLER_P_REG * error / 100) + (CONTROLLER_D_REG * (error - motor->prevError) / 100));

		motor->prevError = error;

		motor->counter = 0;

		return limit8bit(output);

	} else {
		motor->prevError = 0;
		return 0;
	}
}


