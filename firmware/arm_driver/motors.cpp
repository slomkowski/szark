#include "global.h"

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "motors.h"

// mode of operation - DIR, POS, CAL - global
volatile bool calibrated __attribute__ ((section (".noinit")));
volatile bool interruptCalibration, startCalibration;

#define getval(sfr, bit) (bit_is_set(sfr, bit) ? true : false)

// this struct stores the values for single joint
static volatile struct {
	// position
	POSITION currPos; // current position
	POSITION destPos; // desired position
	POSITION maxPos;  // maximal position

	// direction
	DIRECTION allowedDir;
	DIRECTION currDir;

	// speed
	SPEED currSpeed;
	SPEED initSpeed;

	// calibration
	bool calibrated;
	MODE mode;

	// MS parameters
	uint8_t msCounter;bool msCurrState; // false - low, true - high

	// encoders
	bool encoderState;bool prevEncoderState;
} joints[4] __attribute__ ((section (".noinit")));

// these values index the 'joints' table.
typedef enum {
	SHOULDER = 0, ELBOW, GRIPPER, WRIST
} JOINT_INDEX;

//*** local functions
static JOINT_INDEX toJoint(MOTOR motor);
static MOTOR toMotors(JOINT_INDEX index);
static MOTOR toMotors(uint8_t index);
static void setTimerMs(uint16_t interval); // in milliseconds
static void setTimerContinuous();
static bool getMsState(JOINT_INDEX index);
static bool getMsState(uint8_t index);

static volatile bool timerNotClear = true, timerOneShot = true;

//*** basic getters

SPEED motor_get_speed(MOTOR motor) {
	return joints[toJoint(motor)].currSpeed;
}

DIRECTION motor_get_direction(MOTOR motor) {
	return joints[toJoint(motor)].currDir;
}

POSITION motor_get_position(MOTOR motor) {
	return joints[toJoint(motor)].currPos;
}

MODE motor_get_mode(MOTOR motor) {
	return joints[toJoint(motor)].mode;
}

// basic setters. The setters put the data in the structure and send the data to the appropriate output (registers, pins)

void motor_set_speed(MOTOR motor, SPEED speed) {
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

void motor_set_direction(MOTOR motor, DIRECTION direction) {
	DIRECTION currentDir;
	const JOINT_INDEX idx = toJoint(motor);

	if (joints[idx].mode == POS) joints[idx].mode = DIR;

	// put data to the struct, checking if the command is
	if ((((direction == MOTOR_FORWARD) || (direction == MOTOR_BACKWARD)) && (joints[idx].allowedDir == MOTOR_NONE)
		|| (joints[idx].allowedDir == direction))) joints[idx].currDir = direction;

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

void motor_set_position(MOTOR motor, POSITION position) {
	const JOINT_INDEX idx = toJoint(motor);

	if (joints[idx].calibrated == false) return;
	if (joints[idx].maxPos < position) return;

	if ((idx == GRIPPER) && (position > 0)) position = joints[GRIPPER].maxPos;

	joints[idx].destPos = position;

	if (joints[idx].currPos < position) {
		motor_set_direction(motor, MOTOR_FORWARD);
		joints[idx].mode = POS;
	} else if (joints[idx].currPos > position) {
		motor_set_direction(motor, MOTOR_BACKWARD);
		joints[idx].mode = POS;
	} else // if equal
	{
		motor_set_direction(motor, MOTOR_STOP);
		joints[idx].mode = DIR;
	}
}

void motor_brake() {
	uint8_t i;
	// instantly brake all the motors
	for (i = 0; i < 4; i++) {
		motor_set_speed(toMotors(i), 0);
		motor_set_direction(toMotors(i), MOTOR_STOP);
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

void motor_init() {
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
	motor_brake();
	calibrated = false;
}

void motor_calibrate() {
	motor_brake();

	for (uint8_t i = 0; i < 4; i++) {
		motor_set_speed(toMotors(i), joints[i].initSpeed);
		joints[i].allowedDir = MOTOR_NONE;
		joints[i].calibrated = false;
		joints[i].mode = CAL;
	}
	calibrated = false;
	interruptCalibration = false;
	startCalibration = false;

	/// neutral positions: shoulder, gripper - backward, elbow - forward
	// elbow
	if (bit_is_set(PIN(SENSOR_ELBOW_MS_PORT), SENSOR_ELBOW_MS_PIN)) // jeżeli jest na którymś wyłączniku
	motor_set_direction(MOTOR_ELBOW, MOTOR_FORWARD);
	// shoulder
	if (bit_is_set(PIN(SENSOR_SHOULDER_MS_PORT), SENSOR_SHOULDER_MS_PIN)) motor_set_direction(MOTOR_SHOULDER,
		MOTOR_BACKWARD);
	// gripper
	if (bit_is_set(PIN(SENSOR_GRIPPER_MS_PORT), SENSOR_GRIPPER_MS_PIN)) motor_set_direction(MOTOR_GRIPPER,
		MOTOR_BACKWARD);
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

	motor_set_direction(MOTOR_ELBOW, MOTOR_STOP);
	motor_set_direction(MOTOR_SHOULDER, MOTOR_STOP);
	motor_set_direction(MOTOR_GRIPPER, MOTOR_STOP);

	// elbow
	if (getMsState(ELBOW)) motor_set_direction(MOTOR_ELBOW, MOTOR_BACKWARD);
	else motor_set_direction(MOTOR_ELBOW, MOTOR_FORWARD);

	// shoulder
	if (getMsState(SHOULDER)) motor_set_direction(MOTOR_SHOULDER, MOTOR_FORWARD);
	else motor_set_direction(MOTOR_SHOULDER, MOTOR_BACKWARD);

	// gripper
	if (getMsState(GRIPPER)) motor_set_direction(MOTOR_GRIPPER, MOTOR_FORWARD);
	else motor_set_direction(MOTOR_GRIPPER, MOTOR_BACKWARD);

	setTimerMs(700);

	while (timerNotClear && !interruptCalibration)
		;

	motor_set_direction(MOTOR_SHOULDER, MOTOR_BACKWARD);
	motor_set_direction(MOTOR_GRIPPER, MOTOR_BACKWARD);
	motor_set_direction(MOTOR_ELBOW, MOTOR_FORWARD);

	if (interruptCalibration) {
		abortCalibration();
		return;
	}

	setTimerContinuous();

	if (joints[WRIST].calibrated == false) {
		joints[WRIST].currPos = 0;
		motor_set_direction(MOTOR_WRIST, MOTOR_FORWARD);
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
		if ((joints[WRIST].currPos < (POS_MAX_WRIST / 2)) && !backward) motor_set_direction(MOTOR_WRIST, MOTOR_FORWARD);
		if (joints[WRIST].currPos > (POS_MAX_WRIST / 2)) {
			motor_set_direction(MOTOR_WRIST, MOTOR_BACKWARD);
			backward = true;
		}
	}

	else if (joints[WRIST].calibrated == true) {
		if ((joints[WRIST].currPos == 0) && (joints[WRIST].currDir == MOTOR_BACKWARD)) {
			joints[WRIST].allowedDir = MOTOR_FORWARD;
			motor_set_direction(MOTOR_WRIST, MOTOR_STOP);
		} else if ((joints[WRIST].currPos >= POS_MAX_WRIST) && (joints[WRIST].currDir == MOTOR_FORWARD)) {
			joints[WRIST].allowedDir = MOTOR_BACKWARD;
			motor_set_direction(MOTOR_WRIST, MOTOR_STOP);
		} else if ((joints[WRIST].currPos != 0) && (joints[WRIST].currPos != POS_MAX_WRIST)) joints[WRIST].allowedDir =
			MOTOR_NONE;
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
			if (getMsState(idx) && (joints[idx].msCurrState == false)) {
				if (joints[idx].msCounter == MS_DEBOUNCE_COUNTER) {
					if (idx == WRIST) {
						joints[WRIST].currPos = joints[WRIST].maxPos / 2;
						if (joints[WRIST].mode == CAL) {
							motor_set_direction(MOTOR_WRIST, MOTOR_STOP);
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
						motor_set_direction(toMotors(idx), MOTOR_STOP);
					}
					joints[idx].msCurrState = true;
					joints[idx].msCounter = 0;
				} else joints[idx].msCounter++;
			} else if (!getMsState(idx) && (joints[idx].msCurrState == true)) {
				if (joints[idx].msCounter == MS_DEBOUNCE_COUNTER) {
					joints[idx].allowedDir = MOTOR_NONE;

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
						motor_set_direction(toMotors(i), MOTOR_STOP);
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

static JOINT_INDEX toJoint(MOTOR motor) {
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

static MOTOR toMotors(uint8_t index) {
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

static MOTOR toMotors(JOINT_INDEX index) {
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

static bool getMsState(uint8_t index) {
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
