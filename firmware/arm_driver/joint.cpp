#include "global.h"

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "joint.h"

#include "arm_driver-commands.h"

using namespace arm;
using namespace joint;

namespace joint {
// mode of operation - DIR, POS, CAL - global
	volatile bool calibrated __attribute__ ((section (".noinit")));
	volatile bool interruptCalibration, startCalibration;
}

#define getval(sfr, bit) (bit_is_set(sfr, bit) ? true : false)

// this struct stores the values for single joint
static volatile struct {
	// position
	Position currPos; // current position
	Position destPos; // desired position
	Position maxPos;  // maximal position

	// direction
	Direction allowedDir;
	Direction currDir;

	// speed
	Speed currSpeed;
	Speed initSpeed;

	// calibration
	bool calibrated;
	Mode mode;

	// MS parameters
	uint8_t msCounter;
	bool msCurrState; // false - low, true - high

	// encoders
	bool encoderState;
	bool prevEncoderState;
} joints[4] __attribute__ ((section (".noinit")));

// these values index the 'joints' table.
typedef enum {
	SHOULDER = 0, ELBOW, GRIPPER, WRIST
} JOINT_INDEX;

//*** local functions
static JOINT_INDEX toJoint(Motor motor);
static Motor toMotors(uint8_t index);
static void setTimerMs(uint16_t interval); // in milliseconds
static void setTimerContinuous();
static bool getMsState(JOINT_INDEX index);

static volatile bool timerNotClear = true, timerOneShot = true;

//*** basic getters

Speed joint::getSpeed(Motor motor) {
	return joints[toJoint(motor)].currSpeed;
}

Direction joint::getDirection(Motor motor) {
	return joints[toJoint(motor)].currDir;
}

Position joint::getPosition(Motor motor) {
	return joints[toJoint(motor)].currPos;
}

Mode joint::getMode(Motor motor) {
	return joints[toJoint(motor)].mode;
}

// basic setters. The setters put the data in the structure and send the data to the appropriate output (registers, pins)

void joint::setSpeed(Motor motor, Speed speed) {
	// put data to the struct
	joints[toJoint(motor)].currSpeed = speed;

	// put it to the registers
	switch (motor) {
	case MOTOR_SHOULDER:
		PWM_SHOULDER_REG = speed << 4;
		break;
	case MOTOR_ELBOW:
		PWM_ELBOW_REG = speed << 4;
		break;
	case MOTOR_WRIST:
		PWM_WRIST_REG = speed << 4;
		break;
	case MOTOR_GRIPPER:
		PWM_GRIPPER_REG = speed << 4;
		break;
	};
}

void joint::setDirection(Motor motor, Direction direction) {
	Direction currentDir;
	const JOINT_INDEX idx = toJoint(motor);

	if (joints[idx].mode == POS) joints[idx].mode = DIR;

	// put data to the struct, checking if the command is
	if ((((direction == MOTOR_FORWARD) || (direction == MOTOR_BACKWARD)) && (joints[idx].allowedDir == MOTOR_BOTH))
		|| (joints[idx].allowedDir == direction)) joints[idx].currDir = direction;

	else joints[idx].currDir = MOTOR_STOP;

	currentDir = joints[idx].currDir;

	// put them to the output
	switch (motor) {
	case MOTOR_SHOULDER:
		if (currentDir == MOTOR_FORWARD) {
			PORT(MOTOR_SHOULDER_FORWARD_PORT) |= (1 << MOTOR_SHOULDER_FORWARD_PIN);
			PORT(MOTOR_SHOULDER_BACKWARD_PORT) &= ~(1 << MOTOR_SHOULDER_BACKWARD_PIN);
		} else if (currentDir == MOTOR_BACKWARD) {
			PORT(MOTOR_SHOULDER_FORWARD_PORT) &= ~(1 << MOTOR_SHOULDER_FORWARD_PIN);
			PORT(MOTOR_SHOULDER_BACKWARD_PORT) |= (1 << MOTOR_SHOULDER_BACKWARD_PIN);
		} else {
			// braking
			PORT(MOTOR_SHOULDER_FORWARD_PORT) |= (1 << MOTOR_SHOULDER_FORWARD_PIN);
			PORT(MOTOR_SHOULDER_BACKWARD_PORT) |= (1 << MOTOR_SHOULDER_BACKWARD_PIN);
		}
		break;
	case MOTOR_ELBOW:
		if (currentDir == MOTOR_FORWARD) {
			PORT(MOTOR_ELBOW_FORWARD_PORT) |= (1 << MOTOR_ELBOW_FORWARD_PIN);
			PORT(MOTOR_ELBOW_BACKWARD_PORT) &= ~(1 << MOTOR_ELBOW_BACKWARD_PIN);

		} else if (currentDir == MOTOR_BACKWARD) {
			PORT(MOTOR_ELBOW_FORWARD_PORT) &= ~(1 << MOTOR_ELBOW_FORWARD_PIN);
			PORT(MOTOR_ELBOW_BACKWARD_PORT) |= (1 << MOTOR_ELBOW_BACKWARD_PIN);
		} else {
			PORT(MOTOR_ELBOW_FORWARD_PORT) |= (1 << MOTOR_ELBOW_FORWARD_PIN);
			PORT(MOTOR_ELBOW_BACKWARD_PORT) |= (1 << MOTOR_ELBOW_BACKWARD_PIN);
		}
		break;
	case MOTOR_WRIST:
		if (currentDir == MOTOR_FORWARD) {
			PORT(MOTOR_WRIST_FORWARD_PORT) |= (1 << MOTOR_WRIST_FORWARD_PIN);
			PORT(MOTOR_WRIST_BACKWARD_PORT) &= ~(1 << MOTOR_WRIST_BACKWARD_PIN);

		} else if (currentDir == MOTOR_BACKWARD) {
			PORT(MOTOR_WRIST_FORWARD_PORT) &= ~(1 << MOTOR_WRIST_FORWARD_PIN);
			PORT(MOTOR_WRIST_BACKWARD_PORT) |= (1 << MOTOR_WRIST_BACKWARD_PIN);
		} else {
			PORT(MOTOR_WRIST_FORWARD_PORT) |= (1 << MOTOR_WRIST_FORWARD_PIN);
			PORT(MOTOR_WRIST_BACKWARD_PORT) |= (1 << MOTOR_WRIST_BACKWARD_PIN);
		}
		break;
	case MOTOR_GRIPPER:
		if (currentDir == MOTOR_FORWARD) {
			PORT(MOTOR_GRIPPER_FORWARD_PORT) |= (1 << MOTOR_GRIPPER_FORWARD_PIN);
			PORT(MOTOR_GRIPPER_BACKWARD_PORT) &= ~(1 << MOTOR_GRIPPER_BACKWARD_PIN);

		} else if (currentDir == MOTOR_BACKWARD) {
			PORT(MOTOR_GRIPPER_FORWARD_PORT) &= ~(1 << MOTOR_GRIPPER_FORWARD_PIN);
			PORT(MOTOR_GRIPPER_BACKWARD_PORT) |= (1 << MOTOR_GRIPPER_BACKWARD_PIN);
		} else {
			PORT(MOTOR_GRIPPER_FORWARD_PORT) |= (1 << MOTOR_GRIPPER_FORWARD_PIN);
			PORT(MOTOR_GRIPPER_BACKWARD_PORT) |= (1 << MOTOR_GRIPPER_BACKWARD_PIN);
		}
		break;
	};
}

void joint::setPosition(Motor motor, Position position) {
	const JOINT_INDEX idx = toJoint(motor);

	if (joints[idx].calibrated == false) return;
	if (joints[idx].maxPos < position) return;

	if ((idx == GRIPPER) && (position > 0)) position = joints[GRIPPER].maxPos;

	joints[idx].destPos = position;

	if (joints[idx].currPos < position) {
		setDirection(motor, MOTOR_FORWARD);
		joints[idx].mode = POS;
	} else if (joints[idx].currPos > position) {
		setDirection(motor, MOTOR_BACKWARD);
		joints[idx].mode = POS;
	} else // if equal
	{
		setDirection(motor, MOTOR_STOP);
		joints[idx].mode = DIR;
	}
}

void joint::brake() {
	uint8_t i;
	// instantly brake all the motors
	for (i = 0; i < 4; i++) {
		setSpeed(toMotors(i), 0);
		setDirection(toMotors(i), MOTOR_STOP);
	}
}

//*** init functions

static void init_pinouts() {
	DDR(MOTOR_SHOULDER_PWM_PORT) |= (1 << MOTOR_SHOULDER_PWM_PIN);
	DDR(MOTOR_SHOULDER_FORWARD_PORT) |= (1 << MOTOR_SHOULDER_FORWARD_PIN);
	DDR(MOTOR_SHOULDER_BACKWARD_PORT) |= (1 << MOTOR_SHOULDER_BACKWARD_PIN);

	DDR(MOTOR_ELBOW_PWM_PORT) |= (1 << MOTOR_ELBOW_PWM_PIN);
	DDR(MOTOR_ELBOW_FORWARD_PORT) |= (1 << MOTOR_ELBOW_FORWARD_PIN);
	DDR(MOTOR_ELBOW_BACKWARD_PORT) |= (1 << MOTOR_ELBOW_BACKWARD_PIN);

	DDR(MOTOR_WRIST_PWM_PORT) |= (1 << MOTOR_WRIST_PWM_PIN);
	DDR(MOTOR_WRIST_FORWARD_PORT) |= (1 << MOTOR_WRIST_FORWARD_PIN);
	DDR(MOTOR_WRIST_BACKWARD_PORT) |= (1 << MOTOR_WRIST_BACKWARD_PIN);

	DDR(MOTOR_GRIPPER_PWM_PORT) |= (1 << MOTOR_GRIPPER_PWM_PIN);
	DDR(MOTOR_GRIPPER_FORWARD_PORT) |= (1 << MOTOR_GRIPPER_FORWARD_PIN);
	DDR(MOTOR_GRIPPER_BACKWARD_PORT) |= (1 << MOTOR_GRIPPER_BACKWARD_PIN);

	// pwm setup
	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00); // phase-correct
	TCCR0B = (1 << CS01) | (1 << CS00); // prescaler clkio/64

	TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM20); // phase-correct
	TCCR2B = (1 << CS22); // prescaler clkio/64

	// sensors as inputs
	DDR(SENSOR_SHOULDER_ENC_PORT) &= ~(1 << SENSOR_SHOULDER_ENC_PIN);
	DDR(SENSOR_SHOULDER_MS_PORT) &= ~(1 << SENSOR_SHOULDER_MS_PIN);

	DDR(SENSOR_ELBOW_ENC_PORT) &= ~(1 << SENSOR_ELBOW_ENC_PIN);
	DDR(SENSOR_ELBOW_MS_PORT) &= ~(1 << SENSOR_ELBOW_MS_PIN);

	DDR(SENSOR_WRIST_ENC_PORT) &= ~(1 << SENSOR_WRIST_ENC_PIN);
	DDR(SENSOR_WRIST_MS_PORT) &= ~(1 << SENSOR_WRIST_MS_PIN);

	DDR(SENSOR_GRIPPER_ENC_PORT) &= ~(1 << SENSOR_GRIPPER_ENC_PIN);
	DDR(SENSOR_GRIPPER_MS_PORT) &= ~(1 << SENSOR_GRIPPER_MS_PIN);

	// pull-up

	PORT(SENSOR_SHOULDER_ENC_PORT) |= (1 << SENSOR_SHOULDER_ENC_PIN);
	PORT(SENSOR_SHOULDER_MS_PORT) |= (1 << SENSOR_SHOULDER_MS_PIN);

	PORT(SENSOR_ELBOW_ENC_PORT) |= (1 << SENSOR_ELBOW_ENC_PIN);
	PORT(SENSOR_ELBOW_MS_PORT) |= (1 << SENSOR_ELBOW_MS_PIN);

	PORT(SENSOR_WRIST_ENC_PORT) |= (1 << SENSOR_WRIST_ENC_PIN);
	PORT(SENSOR_WRIST_MS_PORT) |= (1 << SENSOR_WRIST_MS_PIN);

	PORT(SENSOR_GRIPPER_ENC_PORT) |= (1 << SENSOR_GRIPPER_ENC_PIN);
	PORT(SENSOR_GRIPPER_MS_PORT) |= (1 << SENSOR_GRIPPER_MS_PIN);

	// enable PCINT interrupts

	//ENCODERS_PCMSK |= (1 << SHOULDER_ENC_PCINT) | (1 << ELBOW_ENC_PCINT) | (1 << WRIST_ENC_PCINT); // there's no gripper position encoder

	// interrupts from switches no longer needed
	//MS_PCMSK |= (1 << SHOULDER_MS_PCINT) | (1 << ELBOW_MS_PCINT) | (1 << WRIST_MS_PCINT) | (1 << GRIPPER_MS_PCINT);

	// interrupt from MS disabled
	//PCICR |= /*(1 << PCIE0) |*/ (1 << PCIE1);
}

static void init_structure() {
	uint8_t i;

	// load max positions
	joints[SHOULDER].maxPos = POS_MAX_SHOULDER;
	joints[ELBOW].maxPos = POS_MAX_ELBOW;
	joints[WRIST].maxPos = POS_MAX_WRIST;
	joints[GRIPPER].maxPos = POS_MAX_GRIPPER;

	joints[SHOULDER].initSpeed = CAL_SPEED_SHOULDER;
	joints[ELBOW].initSpeed = CAL_SPEED_ELBOW;
	joints[WRIST].initSpeed = CAL_SPEED_WRIST;
	joints[GRIPPER].initSpeed = CAL_SPEED_GRIPPER;

	for (i = 0; i < 4; i++) {
		// XXX these should be preserved during reset
		/*joints[i].currPos = joints[i].destPos = 0;
		 joints[i].allowedDir = 0;
		 joints[i].calibrated = false;*/

		joints[i].currDir = MOTOR_STOP;
		joints[i].currSpeed = 0;
		joints[i].mode = DIR;

	}
}

void joint::init() {
	init_pinouts();

	init_structure();

	// XXX preserve during reset
	//calibrated = false;

	startCalibration = false;
	interruptCalibration = false;

	// timer 1 initialisation
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // clkio / 1024

	setTimerContinuous();

	// get values from buttons
	joints[ELBOW].prevEncoderState = joints[ELBOW].encoderState =
		getval(PIN(SENSOR_ELBOW_ENC_PORT), SENSOR_ELBOW_ENC_PIN);
	joints[WRIST].prevEncoderState = joints[WRIST].encoderState =
		getval(PIN(SENSOR_WRIST_ENC_PORT), SENSOR_WRIST_ENC_PIN);
	joints[SHOULDER].prevEncoderState = joints[SHOULDER].encoderState =
		getval(PIN(SENSOR_SHOULDER_ENC_PORT), SENSOR_SHOULDER_ENC_PIN);
}

static void abortCalibration() {
	brake();
	calibrated = false;
}

void joint::calibrate() {
	brake();

	for (uint8_t i = 0; i < 4; i++) {
		setSpeed(toMotors(i), joints[i].initSpeed);
		joints[i].allowedDir = MOTOR_BOTH;
		joints[i].calibrated = false;
		joints[i].mode = CAL;
	}
	calibrated = false;
	interruptCalibration = false;
	startCalibration = false;

	/// neutral positions: shoulder, gripper - backward, elbow - forward
	// elbow
	if (bit_is_set(PIN(SENSOR_ELBOW_MS_PORT), SENSOR_ELBOW_MS_PIN)) // jeżeli jest na którymś wyłączniku
	setDirection(MOTOR_ELBOW, MOTOR_FORWARD);
	// shoulder
	if (bit_is_set(PIN(SENSOR_SHOULDER_MS_PORT), SENSOR_SHOULDER_MS_PIN)) setDirection(MOTOR_SHOULDER, MOTOR_BACKWARD);
	// gripper
	if (bit_is_set(PIN(SENSOR_GRIPPER_MS_PORT), SENSOR_GRIPPER_MS_PIN)) setDirection(MOTOR_GRIPPER, MOTOR_BACKWARD);
	// wrist
	if (bit_is_set(PIN(SENSOR_WRIST_MS_PORT), SENSOR_WRIST_MS_PIN)) {
		joints[WRIST].calibrated = true;
		joints[WRIST].mode = DIR;
	}

	setTimerMs(800);

	while (timerNotClear && !interruptCalibration)
		;

	if (interruptCalibration) // if it was aborted by external signal
	{
		abortCalibration();
		return;
	}

	setDirection(MOTOR_ELBOW, MOTOR_STOP);
	setDirection(MOTOR_SHOULDER, MOTOR_STOP);
	setDirection(MOTOR_GRIPPER, MOTOR_STOP);

	// elbow
	if (getMsState(ELBOW)) setDirection(MOTOR_ELBOW, MOTOR_BACKWARD);
	else setDirection(MOTOR_ELBOW, MOTOR_FORWARD);

	// shoulder
	if (getMsState(SHOULDER)) setDirection(MOTOR_SHOULDER, MOTOR_FORWARD);
	else setDirection(MOTOR_SHOULDER, MOTOR_BACKWARD);

	// gripper
	if (getMsState(GRIPPER)) setDirection(MOTOR_GRIPPER, MOTOR_FORWARD);
	else setDirection(MOTOR_GRIPPER, MOTOR_BACKWARD);

	setTimerMs(700);

	while (timerNotClear && !interruptCalibration)
		;

	setDirection(MOTOR_SHOULDER, MOTOR_BACKWARD);
	setDirection(MOTOR_GRIPPER, MOTOR_BACKWARD);
	setDirection(MOTOR_ELBOW, MOTOR_FORWARD);

	if (interruptCalibration) {
		abortCalibration();
		return;
	}

	setTimerContinuous();

	if (joints[WRIST].calibrated == false) {
		joints[WRIST].currPos = 0;
		setDirection(MOTOR_WRIST, MOTOR_FORWARD);
		joints[WRIST].mode = CAL;
	}

	// wait till all the joints are calibrated
	while (!interruptCalibration) {
		if (joints[ELBOW].calibrated && joints[SHOULDER].calibrated && joints[WRIST].calibrated
			&& joints[GRIPPER].calibrated) break;
	}

	if (interruptCalibration) {
		abortCalibration();
		return;
	}

	joints[SHOULDER].currPos = 0;
	joints[GRIPPER].currPos = 0;
	joints[ELBOW].currPos = joints[ELBOW].maxPos;
	joints[WRIST].currPos = joints[WRIST].maxPos / 2;

	for (uint8_t i = 0; i < 4; i++)
		joints[i].mode = DIR;

	calibrated = true;
}

static inline void wristEncoderSupport() {
//	if(joints[WRIST].calibrated)
	static bool backward = false;

	if (joints[WRIST].mode == CAL) {
		if ((joints[WRIST].currPos < (POS_MAX_WRIST / 2)) && !backward) setDirection(MOTOR_WRIST, MOTOR_FORWARD);
		if (joints[WRIST].currPos > (POS_MAX_WRIST / 2)) {
			setDirection(MOTOR_WRIST, MOTOR_BACKWARD);
			backward = true;
		}
	}

	else if (joints[WRIST].calibrated == true) {
		if ((joints[WRIST].currPos == 0) && (joints[WRIST].currDir == MOTOR_BACKWARD)) {
			joints[WRIST].allowedDir = MOTOR_FORWARD;
			setDirection(MOTOR_WRIST, MOTOR_STOP);
		} else if ((joints[WRIST].currPos >= POS_MAX_WRIST) && (joints[WRIST].currDir == MOTOR_FORWARD)) {
			joints[WRIST].allowedDir = MOTOR_BACKWARD;
			setDirection(MOTOR_WRIST, MOTOR_STOP);
		} else if ((joints[WRIST].currPos != 0) && (joints[WRIST].currPos != POS_MAX_WRIST)) joints[WRIST].allowedDir =
			MOTOR_BOTH;
	}
}

// timer interrupt
ISR(TIMER1_COMPA_vect) {
	if (timerOneShot) // this is used when calibration
	{
		TIMSK1 &= ~(1 << OCIE1A); // disable timer interrupt
		timerNotClear = false;
	} else {
		// standard; one tick per 2 ms
		for (uint8_t idx = 0; idx < 4; idx++) {
			if (getMsState((JOINT_INDEX) idx) && (joints[idx].msCurrState == false)) {
				if (joints[idx].msCounter == MS_DEBOUNCE_COUNTER) {
					if (idx == WRIST) {
						joints[WRIST].currPos = joints[WRIST].maxPos / 2;
						if (joints[WRIST].mode == CAL) {
							setDirection(MOTOR_WRIST, MOTOR_STOP);
							joints[WRIST].mode = DIR;
						}
						joints[WRIST].calibrated = true;
					} else {
						joints[idx].calibrated = true;

						if (joints[idx].currDir == MOTOR_BACKWARD) {
							joints[idx].allowedDir = MOTOR_FORWARD;
							joints[idx].currPos = 0;
						} else if (joints[idx].currDir == MOTOR_FORWARD) {
							joints[idx].allowedDir = MOTOR_BACKWARD;
							joints[idx].currPos = joints[idx].maxPos;
						}
						setDirection(toMotors(idx), MOTOR_STOP);
					}
					joints[idx].msCurrState = true;
					joints[idx].msCounter = 0;
				} else joints[idx].msCounter++;
			} else if (!getMsState((JOINT_INDEX) idx) && (joints[idx].msCurrState == true)) {
				if (joints[idx].msCounter == MS_DEBOUNCE_COUNTER) {
					joints[idx].allowedDir = MOTOR_BOTH;

					if (idx == GRIPPER) joints[GRIPPER].currPos = joints[GRIPPER].maxPos;

					joints[idx].msCurrState = false;
					joints[idx].msCounter = 0;
				} else joints[idx].msCounter++;
			}
		}

		// encoders
		for (uint8_t i = 0; i < 4; i++)
			joints[i].prevEncoderState = joints[i].encoderState;

		joints[ELBOW].encoderState = getval(PIN(SENSOR_ELBOW_ENC_PORT), SENSOR_ELBOW_ENC_PIN);
		joints[WRIST].encoderState = getval(PIN(SENSOR_WRIST_ENC_PORT), SENSOR_WRIST_ENC_PIN);
		joints[SHOULDER].encoderState = getval(PIN(SENSOR_SHOULDER_ENC_PORT), SENSOR_SHOULDER_ENC_PIN);

		for (uint8_t i = 0; i < 4; i++)
			if (joints[i].prevEncoderState != joints[i].encoderState) {
				if (i == GRIPPER) continue;

				if (joints[i].mode == POS) {
					if (joints[i].destPos == joints[i].currPos) {
						setDirection(toMotors(i), MOTOR_STOP);
						joints[i].mode = DIR;
					}
				}

				// with limit values to maxPos and zero
				if (joints[i].currDir == MOTOR_FORWARD) joints[i].currPos =
					joints[i].currPos >= joints[i].maxPos ? joints[i].maxPos : joints[i].currPos + 1;
				else if (joints[i].currDir == MOTOR_BACKWARD) joints[i].currPos =
					joints[i].currPos == 0 ? 0 : joints[i].currPos - 1;

				if (i == WRIST) wristEncoderSupport();
			}
	}
}

//*** local auxiliary functions

static JOINT_INDEX toJoint(Motor motor) {
	switch (motor) {
	case MOTOR_SHOULDER:
		return SHOULDER;
	case MOTOR_GRIPPER:
		return GRIPPER;
	case MOTOR_WRIST:
		return WRIST;
	case MOTOR_ELBOW:
	default:
		return ELBOW;
	}
}

static Motor toMotors(uint8_t index) {
	switch (index) {
	case SHOULDER:
		return MOTOR_SHOULDER;
	case GRIPPER:
		return MOTOR_GRIPPER;
	case WRIST:
		return MOTOR_WRIST;
	case ELBOW:
	default:
		return MOTOR_ELBOW;
	}
}

static void setTimerMs(uint16_t interval) {
	timerNotClear = true;
	timerOneShot = true;

	// one tick is ca. 0.128 ms
	OCR1A = interval / 0.128; // put upper value in capture register
	TCNT1 = 0; // clear counter
	TIMSK1 |= (1 << OCIE1A); // enable timer interrupt
}

static void setTimerContinuous() {
	timerOneShot = false;
	OCR1A = TIMER_MS_INTERVAL / 0.128; // put upper value in capture register
	TCNT1 = 0; // clear counter
	TIMSK1 |= (1 << OCIE1A); // enable timer interrupt
}

static bool getMsState(JOINT_INDEX index) {
	uint8_t val;
	switch (index) {
	case SHOULDER:
		val = bit_is_set(PIN(SENSOR_SHOULDER_MS_PORT), SENSOR_SHOULDER_MS_PIN);
		break;
	case GRIPPER:
		val = bit_is_set(PIN(SENSOR_GRIPPER_MS_PORT), SENSOR_GRIPPER_MS_PIN);
		break;
	case WRIST:
		val = bit_is_set(PIN(SENSOR_WRIST_MS_PORT), SENSOR_WRIST_MS_PIN);
		break;
	case ELBOW:
		val = bit_is_set(PIN(SENSOR_ELBOW_MS_PORT), SENSOR_ELBOW_MS_PIN);
		break;
	default:
		return false;
	}

	return val ? true : false;
}

