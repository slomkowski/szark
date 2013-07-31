#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <pthread.h>
#include "config.h"

// used in MotorParams
typedef enum { D_STOP = '0', D_FORWARD = 'f', D_BACKWARD = 'b' } DIRECTION;
typedef unsigned char SPEED;

class MotorParams
{
public:
	SPEED speed;
	DIRECTION direction;

	MotorParams()
	{
		speed = 0;
		direction = D_STOP;
	}
};

// TODO that will be probably changed
typedef unsigned char Position;

// struct to store commands, which are send to the device
/* 
   - motors
   - lcd display
   - lights
   - robot arm
   - server commands (exit, reboot, emergency reset)
*/

class SZARKCommand
{
public:
	//// !!! data 

	// motors
	struct
	{
		MotorParams left, right;
	} motors;

	// arm
	struct
	{
		MotorParams gripper, elbow, wrist, shoulder;
	} arm;

	// this is used to switch arm mode to 'position set mode'
	struct
	{
		Position shoulder, elbow, wrist, gripper;
	} armPositions; // where the desired position are set

	bool armGoToPos; // trigger, it will be cleared after the command are sent to device

	// lights'n'i2c stuff
	struct
	{
		bool low;
		bool high;
		bool gripper;
		bool camera;
	} lights;
	
	// LCD display
	char lcdText[LCD_CHARACTERS + 1]; // +1 for \0

	// server commands
	bool performEmergencyStop;
	bool performExit;

	//// !!! methods 
	
	void copy(SZARKCommand source, bool nolock = false);

	void init();

	// commands affect internal mutex
	void lock();
	void unlock();

	SZARKCommand();

private:
	pthread_mutex_t mutex;	
};

enum Runlevel { OPERATIONAL = 0, STOPPED_SOFT, STOPPED_HARD };

// struct holds data received from the device
class SZARKStatus
{
public:
	// battery
	struct
	{
		float voltage;
		float current;
	} battery;

	// arm actual position and direction
	struct
	{
		Position shoulder, elbow, wrist, gripper;
	} armPositions; 
	struct
	{
		DIRECTION shoulder, elbow, wrist, gripper;
	} armDirections;

	Runlevel runlevel; // this stores the mode, allowed: OPERATIONAL, STOPPED_HARD, STOPPED_SOFT

	//// !!! methods
	
	void copy(SZARKStatus source);
	void init();

	void lock();
	void unlock();

	SZARKStatus();

private:
	pthread_mutex_t mutex;	
};

#endif

