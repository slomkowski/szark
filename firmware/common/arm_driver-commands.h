#ifndef __ARM_COMMAND_CHARS_H__
#define __ARM_COMMAND_CHARS_H__

/** Description of the protocol used in the communication with the arm driver. Data is sent by I2C interface.
 This file is included to 'bridge' during compilation.
 */

/* definition on the characters used in protocol */

#define CHAR_ARM_GET_SPEED		'g'
#define CHAR_ARM_SET_SPEED		's'

#define CHAR_ARM_SET_DIRECTION	'd'
#define CHAR_ARM_GET_DIRECTION	't'

#define CHAR_ARM_GET_POSITION		'q'
#define CHAR_ARM_SET_POSITION		'p'

#define CHAR_ARM_BRAKE			'B'
#define CHAR_ARM_CALIBRATE		'C'
#define CHAR_ARM_GET_MODE		'M'

#define CHAR_ARM_IS_CALIBRATED	'K'

#define CHAR_ARM_TRUE			't'
#define CHAR_ARM_FALSE			'f'

namespace arm {
	typedef enum {
		MOTOR_STOP = '0', MOTOR_FORWARD = 'f', MOTOR_BACKWARD = 'b', MOTOR_BOTH = 0
	} Direction;

	typedef enum {
		MOTOR_SHOULDER = 's', MOTOR_ELBOW = 'e', MOTOR_WRIST = 'w', MOTOR_GRIPPER = 'g'
	} Motor;

	typedef enum {
		DIR = 'D', POS = 'P', CAL = 'C'
	} Mode;
}

#endif

