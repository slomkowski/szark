#ifndef _PWM_H_
#define _PWM_H_

#include "global.h"
#include <inttypes.h>
#include <avr/io.h>

// ** I/O configuration
#define MOTOR1_PORT			B
#define MOTOR1_FORWARD_PIN	1
#define MOTOR1_BACKWARD_PIN	0
#define MOTOR1_PWM_PIN		2

#define MOTOR2_PORT			D
#define MOTOR2_FORWARD_PIN	4
#define MOTOR2_BACKWARD_PIN	6
#define MOTOR2_PWM_PIN		5

#define ENCODER_PORT	D
#define ENCODER1_PIN	2
#define ENCODER2_PIN	3

// ** external interrupts used by encoders
#define ENCODER1_INTERRUPT	INT0_vect
#define ENCODER2_INTERRUPT	INT1_vect

// ** control commands
#define MOTOR_STOP		CHAR_MOTOR_STOP
#define MOTOR_FORWARD	CHAR_MOTOR_FORWARD
#define MOTOR_BACKWARD	CHAR_MOTOR_BACKWARD

#define MOTOR1_IDENTIFIER CHAR_MOTOR1
#define MOTOR2_IDENTIFIER CHAR_MOTOR2

#define PWM1_REG OCR0A
#define PWM2_REG OCR0B

namespace motor {
	void init();

	uint8_t getSpeed(uint8_t motor);
	uint8_t getDirection(uint8_t motor);

	void setSpeed(uint8_t motor, uint8_t speed);
	void setDirection(uint8_t motor, uint8_t direction);

	enum Direction {
		STOP = CHAR_MOTOR_STOP, FORWARD = CHAR_MOTOR_FORWARD, BACKWARD = CHAR_MOTOR_BACKWARD
	};

	struct Motor {
		Direction direction;
		volatile uint8_t refSpeed;
		volatile uint16_t counter;
		int16_t prevError;
		uint8_t fixedDriveCycleCounter;
	};

	extern Motor motor1;
	extern Motor motor2;
}

#endif
