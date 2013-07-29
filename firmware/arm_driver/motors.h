#ifndef _MOTORS_H_
#define _MOTORS_H_

#include <inttypes.h>

#include "global.h"
#include "command-chars.h"


//*** I/O configuration - bound to hardware

// output pins
#define MOTOR_GRIPPER_PWM_PORT D
#define MOTOR_GRIPPER_PWM_PIN 6
#define MOTOR_GRIPPER_FORWARD_PORT B
#define MOTOR_GRIPPER_FORWARD_PIN 0
#define MOTOR_GRIPPER_BACKWARD_PORT D
#define MOTOR_GRIPPER_BACKWARD_PIN 7

#define MOTOR_SHOULDER_PWM_PORT B
#define MOTOR_SHOULDER_PWM_PIN 3
#define MOTOR_SHOULDER_FORWARD_PORT D
#define MOTOR_SHOULDER_FORWARD_PIN 0
#define MOTOR_SHOULDER_BACKWARD_PORT D
#define MOTOR_SHOULDER_BACKWARD_PIN 1

#define MOTOR_WRIST_PWM_PORT D
#define MOTOR_WRIST_PWM_PIN 5
#define MOTOR_WRIST_FORWARD_PORT B
#define MOTOR_WRIST_FORWARD_PIN 6
#define MOTOR_WRIST_BACKWARD_PORT B
#define MOTOR_WRIST_BACKWARD_PIN 7

#define MOTOR_ELBOW_PWM_PORT D
#define MOTOR_ELBOW_PWM_PIN 3
#define MOTOR_ELBOW_FORWARD_PORT D
#define MOTOR_ELBOW_FORWARD_PIN 2
#define MOTOR_ELBOW_BACKWARD_PORT D
#define MOTOR_ELBOW_BACKWARD_PIN 4

#define PWM_GRIPPER_REG OCR0A
#define PWM_WRIST_REG OCR0B
#define PWM_SHOULDER_REG OCR2A
#define PWM_ELBOW_REG  OCR2B

// sensors
#define SENSOR_SHOULDER_ENC_PORT C
#define SENSOR_SHOULDER_ENC_PIN 0
#define SENSOR_SHOULDER_MS_PORT B
#define SENSOR_SHOULDER_MS_PIN 5

#define SENSOR_ELBOW_ENC_PORT C
#define SENSOR_ELBOW_ENC_PIN 1
#define SENSOR_ELBOW_MS_PORT B
#define SENSOR_ELBOW_MS_PIN 4

#define SENSOR_WRIST_ENC_PORT C
#define SENSOR_WRIST_ENC_PIN 2
#define SENSOR_WRIST_MS_PORT B
#define SENSOR_WRIST_MS_PIN 2

#define SENSOR_GRIPPER_ENC_PORT C
#define SENSOR_GRIPPER_ENC_PIN 3
#define SENSOR_GRIPPER_MS_PORT B
#define SENSOR_GRIPPER_MS_PIN 1

#define ENCODERS_PCMSK PCMSK1
#define SHOULDER_ENC_PCINT PCINT8
#define ELBOW_ENC_PCINT PCINT9
#define WRIST_ENC_PCINT PCINT10
#define GRIPPER_ENC_PCINT PCINT11

#define MS_PCMSK PCMSK0
#define SHOULDER_MS_PCINT PCINT5
#define ELBOW_MS_PCINT PCINT4
#define WRIST_MS_PCINT PCINT2
#define GRIPPER_MS_PCINT PCINT1

#define ENCODERS_INTERRUPT PCINT1_vect
#define MS_INTERRUPT PCINT0_vect

//*** initial speed settings - range 0 - 15
#define CAL_SPEED_SHOULDER	220
#define CAL_SPEED_ELBOW		100 
#define CAL_SPEED_WRIST		100
#define CAL_SPEED_GRIPPER	15

#define POS_MAX_SHOULDER	79
#define POS_MAX_ELBOW	105
#define POS_MAX_WRIST	95
#define POS_MAX_GRIPPER	255

#define MS_DEBOUNCE_TIME 10 // ms
#define TIMER_MS_INTERVAL 2 // ms

#define MS_DEBOUNCE_COUNTER (MS_DEBOUNCE_TIME / TIMER_MS_INTERVAL)

//*** type definitions
typedef uint8_t POSITION;
typedef uint8_t SPEED;
// typedef DIRECTION - definition is in command-chars.h

typedef enum { DIR = 'D', POS = 'P', CAL = 'C' } MODE;
extern volatile bool startCalibration;
extern volatile bool calibrated __attribute__ ((section (".noinit")));
extern volatile bool interruptCalibration;

//*** getters
SPEED motor_get_speed(MOTOR motor);
DIRECTION motor_get_direction(MOTOR motor);
POSITION motor_get_position(MOTOR motor);
MODE motor_get_mode(MOTOR motor);

//*** setters
void motor_set_speed(MOTOR motor, SPEED speed);
void motor_set_direction(MOTOR motor, DIRECTION direction);
void motor_set_position(MOTOR motor, POSITION position);

//*** additional functions
void motor_calibrate();
void motor_brake();
void motor_init();


#endif
