#ifndef __ARM_COMMAND_CHARS_H__
#define __ARM_COMMAND_CHARS_H__

/** Description of the protocol used in the communication with the arm driver. Data is sent by I2C interface.
 This file is included to 'bridge' and 'arm_driver' during compilation.
 */

#include <inttypes.h>

namespace arm {

	enum Command
		:uint8_t {
			GET_SPEED = 1,
		SET_SPEED,
		SET_DIRECTION,
		GET_DIRECTION,
		SET_POSITION,
		GET_POSITION,
		BRAKE,
		CALIBRATE,
		GET_MODE,
		IS_CALIBRATED
	};

	enum BooleanVals
		: uint8_t {
			FALSE, TRUE
	};

	enum Direction
		:uint8_t {
			STOP, FORWARD, BACKWARD, BOTH
	};

	enum Motor
		:uint8_t {
			SHOULDER, ELBOW, WRIST, GRIPPER
	};

	enum Mode
		:uint8_t {
			DIR, POS, CAL
	};
}

#endif

