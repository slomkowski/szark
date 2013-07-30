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

/* definition on the characters used in protocol */

#define CHAR_MOTOR_GET_SPEED		'g'
#define CHAR_MOTOR_SET_SPEED		's'
#define CHAR_MOTOR_SET_DIRECTION	'd'
#define CHAR_MOTOR_GET_DIRECTION	't'
#define CHAR_MOTOR_BRAKE			'B'

#define CHAR_MOTOR1		'l'
#define CHAR_MOTOR2		'r'

// setting direction
#define CHAR_MOTOR_STOP		'0'
#define CHAR_MOTOR_FORWARD		'f'
#define CHAR_MOTOR_BACKWARD	'b'

namespace motor {
	enum Direction {
		STOP = CHAR_MOTOR_STOP, FORWARD = CHAR_MOTOR_FORWARD, BACKWARD = CHAR_MOTOR_BACKWARD
	};

	enum Motor {
		MOTOR1 = CHAR_MOTOR1, MOTOR2 = CHAR_MOTOR2
	};
}

#endif

