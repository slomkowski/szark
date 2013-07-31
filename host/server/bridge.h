#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include "structures.h"

int b_setup(const char *device, int baudrate);
void b_shutdown();

void b_enable();
void b_disable(bool msg = false);

void b_setLeftMotor(MotorParams m);
void b_setLeftMotorDirection(DIRECTION d);
void b_setLeftMotorSpeed(SPEED speed);

void b_setRightMotor(MotorParams m);
void b_setRightMotorDirection(DIRECTION d);
void b_setRightMotorSpeed(SPEED speed);

void b_setLightLow(bool enable);
void b_setLightHigh(bool enable);
void b_setLightGripper(bool enable);
void b_setLightCamera(bool enable);

void b_setLCDText(const char *text);

void b_battery_measure();

void *b_data_listener_function(void* arg);
void *b_data_sender_function(void *arg);

void b_getArmAllDirections();
void b_getArmAllPositions();
void b_setArmWristPosition(Position pos);
void b_setArmElbowPosition(Position pos);
void b_setArmShoulderPosition(Position pos);
void b_setArmGripperPosition(Position pos);
void b_setArmGripper(MotorParams m);
void b_setArmGripperSpeed(SPEED speed);
void b_setArmGripperDirection(DIRECTION d);
void b_setArmWrist(MotorParams m);
void b_setArmWristSpeed(SPEED speed);
void b_setArmWristDirection(DIRECTION d);
void b_setArmElbow(MotorParams m);
void b_setArmElbowSpeed(SPEED speed);
void b_setArmElbowDirection(DIRECTION d);
void b_setArmShoulder(MotorParams m);
void b_setArmShoulderSpeed(SPEED speed);
void b_setArmShoulderDirection(DIRECTION d);

void b_getAllData();

#define MESSAGE_READY_TO_COMMAND "Machine ready to$command."

// received
#define RS_INITIALIZATION 'I'
#define RS_HARDWARE_STOP 'E'
#define RS_STOP 'S'

#define RS_EMERGENCY_STOP_STRING "SSSSSSSSS"
#define RS_INIT_CHAR 'I'

#define RS_LCD_WRITE 'W'
#define RS_LCD_NEWLINE '$'
#define RS_LCD_EOF '~'

#define RS_BATTERY 'B'

#define RS_EXPANDER 'L'
#define RS_EXPANDER_SET 's'
#define RS_LIGHT_GRIPPER 7
#define RS_LIGHT_HIGH 3
#define RS_LIGHT_LOW 2
#define RS_LIGHT_CAMERA 6
// TODO upewnić, się, że oświetlenie kamery jest właśnie tam podłączone

// sent
#define RS_BUTTON_ESC 'X'
#define RS_BUTTON_NEXT 'Y'
#define RS_BUTTON_ENTER 'Z'

#define RS_M_FORWARD 'f'
#define RS_M_BACKWARD 'b'
#define RS_M_STOP '0'

#define RS_A_FORWARD RS_M_FORWARD
#define RS_A_BACKWARD RS_M_BACKWARD
#define RS_A_STOP RS_M_STOP

// battery configuration
#define BATTERY_VOLTAGE_DIVIDING_FACTOR 56.42
#define BATTERY_CURRENT_DIVIDING_FACTOR 57.7087

#define RS_ARM 'A'
#define RS_ARM_GET_ALL_DIRECTIONS 'D'
#define RS_ARM_GET_ALL_POSITIONS 'P'

#endif
