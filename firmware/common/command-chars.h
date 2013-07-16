#ifndef __COMMAND_CHARS_H__
#define __COMMAND_CHARS_H__

/** Description of the protocol used in the communication with the motor driver. Data is sent by I2C interface.
  
  - setting direction: START|address|CHAR_MOTOR_SET_DIRECTION|motor no.|value|STOP
     where motor no. is CHAR_MOTOR1 or CHAR_MOTOR2, value is CHAR_MOTOR_STOP/CHAR_MOTOR_BACKWARD/CHAR_MOTOR_FORWARD
  - setting speed: START|address|CHAR_MOTOR_SET_SPEED|motor no.|value|STOP
     where value is 0 - 255
  - getting speed: START|
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
#define CHAR_MOTOR_FORWARD	'f'
#define CHAR_MOTOR_BACKWARD	'b'

#endif

