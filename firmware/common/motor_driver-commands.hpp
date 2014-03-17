#ifndef __COMMAND_CHARS_H__
#define __COMMAND_CHARS_H__

/** Description of the protocol used in the communication with the motor driver. Data is sent by I2C interface.

 - setting direction: START|address|CHAR_MOTOR_SET_DIRECTION|motor no.|value|STOP
 where motor no. is CHAR_MOTOR1 or CHAR_MOTOR2, value is CHAR_MOTOR_STOP/CHAR_MOTOR_BACKWARD/CHAR_MOTOR_FORWARD
 - setting speed: START|address|CHAR_MOTOR_SET_SPEED|motor no.|value|STOP
 where value is 0 - 255
 - getting speed: START|address|CHAR_MOTOR_GET_SPEED|motor no.|STOP
 The same is for direction.
 - braking: START|address|CHAR_MOTOR_BRAKE|STOP
 */

#include <inttypes.h>

namespace motor {
	enum Command
		:uint8_t {
			GET_SPEED = 1, SET_SPEED, GET_DIRECTION, SET_DIRECTION, BRAKE
	};

	enum Direction
		:uint8_t {
			STOP, FORWARD, BACKWARD
	};

	enum Motor
		:uint8_t {
			MOTOR1, MOTOR2
	};
}

#endif

