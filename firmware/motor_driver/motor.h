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

#define PWM1_REG OCR0A
#define PWM2_REG OCR0B

#include "motor_driver-commands.h"

namespace motor {
	void init();

	uint8_t getSpeed(Motor motor);
	Direction getDirection(Motor motor);

	void setSpeed(Motor motor, uint8_t speed);
	void setDirection(Motor motor, Direction direction);

	struct MotorStruct {
		Direction direction;
		volatile uint8_t refSpeed;
		volatile uint16_t counter;
		int16_t prevError;
		uint8_t fixedDriveCycleCounter;
	};

	extern MotorStruct motor1;
	extern MotorStruct motor2;
}

#endif
