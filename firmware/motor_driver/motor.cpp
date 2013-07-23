#include "global.h"

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/pgmspace.h>

#include <stdlib.h>

#include "motor.h"
#include "controller.h"

/* Interrupt interval - after this time the PD controller interrupt is called.
 * The base is (f_clk/1024) ~= 0.25ms. Current setting 500 = 125ms
 */
const uint16_t TIMER_INTERVAL = 500;

/*
 * After each change of direction or speed the motor's PWM value is taken
 * from Mx_SPEEDS array and held for the several timer cycles. After that
 * the PD controller is enabled.
 */
const uint8_t FIXED_DRIVE_CYCLES = 3;

/*
 * in my application, the maximal controlled speed was limited to 92 impulses per interrupt
 * this is the table of valid speeds (number of impulses in TIMER_INTERVAL time). There are
 * 12 speed levels (from 0 to 11). 11 is maximal possible speed (the motors are driven with
 * full supply voltage).
 */
const uint8_t REF_SPEEDS[] PROGMEM = { 0, 10, 20, 28, 36, 46, 54, 66, 74, 84, 92, 255 };

/*
 * The fixed PWM values for each motor for the static drive phase (info above).
 */
const uint8_t M1_SPEEDS[] PROGMEM = { 0, 40, 50, 60, 75, 100, 130, 160, 180, 200, 230, 255 };
const uint8_t M2_SPEEDS[] PROGMEM = { 0, 65, 75, 90, 100, 125, 150, 170, 190, 215, 240, 255 };

static const uint8_t NO_SPEEDS = sizeof(REF_SPEEDS) / sizeof(REF_SPEEDS[0]);

namespace motor {
	Motor motor1;
	Motor motor2;
}

using namespace motor;

void motor::init() {
	// set outputs
	DDR(MOTOR1_PORT) |= (1 << MOTOR1_FORWARD_PIN) | (1 << MOTOR1_BACKWARD_PIN) | (1 << MOTOR1_PWM_PIN);
	DDR(MOTOR2_PORT) |= (1 << MOTOR2_FORWARD_PIN) | (1 << MOTOR2_BACKWARD_PIN) | (1 << MOTOR2_PWM_PIN);

	//set encoders inputs & pullups
	DDR(ENCODER_PORT) &= ~(1 << ENCODER1_PIN) & ~(1 << ENCODER2_PIN);
	PORT(ENCODER_PORT) |= (1 << ENCODER1_PIN) | (1 << ENCODER2_PIN);

	// brake motors
	setDirection(MOTOR1_IDENTIFIER, MOTOR_STOP);
	setDirection(MOTOR2_IDENTIFIER, MOTOR_STOP);

	// pwm setup
	TCCR0A = (1 << WGM00); // phase-correct
	TCCR0B = (1 << CS01) | (1 << CS00); // prescaler clkio/64

	// external interrupts setup - for encoders support
	MCUCR |= (1 << ISC10) | (1 << ISC00); // enable interrupts at both rising and falling edges

	// setup of the interrupt timer,
	// Timer1 -- CTC mode, enable overflow interrupt, prescaler f_clk/1024
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);

	TIMSK |= (1 << OCIE1A);
	OCR1A = TIMER_INTERVAL;

	// enable encoders interrupts
	GIMSK |= (1 << INT0) | (1 << INT1);
}

/*
 * Returned value is the actual PWM setting, not the value set by user.
 */
uint8_t motor::getSpeed(uint8_t motor) {
	if (motor == MOTOR1_IDENTIFIER) {
		return PWM1_REG ;
	} else {
		return PWM2_REG ;
	}
}

uint8_t motor::getDirection(uint8_t motor) {
	if (motor == MOTOR1_IDENTIFIER) {
		return (motor1.direction);
	} else {
		return (motor2.direction);
	}
}

void motor::setSpeed(uint8_t motor, uint8_t speed) {
	uint8_t newRefSpeed;

#if DEBUG
	if(speed == '0') newRefSpeed = pgm_read_byte(&(REF_SPEEDS[0]));
	else if(speed == 'm') newRefSpeed = pgm_read_byte(&(REF_SPEEDS[NO_SPEEDS - 1]));
	else
#endif
	newRefSpeed = pgm_read_byte(&(REF_SPEEDS[speed % NO_SPEEDS]));

	if (motor == MOTOR1_IDENTIFIER) {
		if (newRefSpeed != motor1.refSpeed) {
			PWM1_REG = pgm_read_byte(&(M1_SPEEDS[speed % NO_SPEEDS]));
			motor1.fixedDriveCycleCounter = FIXED_DRIVE_CYCLES;
		}
	} else {
		if (newRefSpeed != motor2.refSpeed) {
			PWM2_REG = pgm_read_byte(&(M2_SPEEDS[speed % NO_SPEEDS]));
			motor2.fixedDriveCycleCounter = FIXED_DRIVE_CYCLES;
		}
	}
}

static void updateDirectionPins(uint8_t motor, uint8_t direction) {
	if (motor == MOTOR1_IDENTIFIER) {
		switch (direction) {
		case MOTOR_FORWARD:
			PORT(MOTOR1_PORT) |= (1 << MOTOR1_FORWARD_PIN);
			PORT(MOTOR1_PORT) &= ~(1 << MOTOR1_BACKWARD_PIN);
			break;
		case MOTOR_BACKWARD:
			PORT(MOTOR1_PORT) &= ~(1 << MOTOR1_FORWARD_PIN);
			PORT(MOTOR1_PORT) |= (1 << MOTOR1_BACKWARD_PIN);
			break;
		default:
			PORT(MOTOR1_PORT) |= (1 << MOTOR1_FORWARD_PIN);
			PORT(MOTOR1_PORT) |= (1 << MOTOR1_BACKWARD_PIN);
			break;
		}
	} else {
		switch (direction) {
		case MOTOR_FORWARD:
			PORT(MOTOR2_PORT) |= (1 << MOTOR2_FORWARD_PIN);
			PORT(MOTOR2_PORT) &= ~(1 << MOTOR2_BACKWARD_PIN);
			break;
		case MOTOR_BACKWARD:
			PORT(MOTOR2_PORT) &= ~(1 << MOTOR2_FORWARD_PIN);
			PORT(MOTOR2_PORT) |= (1 << MOTOR2_BACKWARD_PIN);
			break;
		default:
			PORT(MOTOR2_PORT) |= (1 << MOTOR2_FORWARD_PIN);
			PORT(MOTOR2_PORT) |= (1 << MOTOR2_BACKWARD_PIN);
			break;
		}
	}
}

void motor::setDirection(uint8_t motor, uint8_t direction = MOTOR_STOP) {
	uint8_t reg;
	Motor *mStruct;
	Direction newDirection;

	if (motor == MOTOR1_IDENTIFIER) {
		reg = (1 << COM0A1);
		mStruct = &motor1;
	} else {
		reg = (1 << COM0B1);
		mStruct = &motor2;
	}

	switch (direction) {
	case MOTOR_FORWARD:
		TCCR0A |= reg;
		newDirection = FORWARD;
		break;
	case MOTOR_BACKWARD:
		TCCR0A |= reg;
		newDirection = BACKWARD;
		break;
	default:
		TCCR0A &= ~reg;
		newDirection = STOP;
		break;
	};

	uint8_t speedIdx = 0;
	for (uint8_t i = 0; i < NO_SPEEDS; i++) {
		if (pgm_read_byte(&(REF_SPEEDS[i])) == mStruct->refSpeed) {
			speedIdx = i;
			break;
		}
	}

	if (newDirection != mStruct->direction) {
		if (motor == MOTOR1_IDENTIFIER) {
			PWM1_REG = pgm_read_byte(&(M1_SPEEDS[speedIdx]));
		} else {
			PWM2_REG = pgm_read_byte(&(M2_SPEEDS[speedIdx]));
		}
		mStruct->fixedDriveCycleCounter = FIXED_DRIVE_CYCLES;
		mStruct->direction = newDirection;
		mStruct->counter = 0;

		updateDirectionPins(motor, direction);
	}
}

ISR(ENCODER1_INTERRUPT) {
	motor1.counter++;
}

ISR(ENCODER2_INTERRUPT) {
	motor2.counter++;
}

ISR(TIMER1_COMPA_vect) {
	if (motor1.fixedDriveCycleCounter == 0) {
		PWM1_REG = regulate(&motor1, PWM1_REG );
	} else {
		motor1.fixedDriveCycleCounter--;
	}
	if (motor2.fixedDriveCycleCounter == 0) {
		PWM2_REG = regulate(&motor2, PWM2_REG );
	} else {
		motor2.fixedDriveCycleCounter--;
	}
}
