#include <string.h>
#include <pthread.h>
#include "structures.h"
#include "extra_func.h"
#include <stdio.h>

SZARKCommand::SZARKCommand()
{
	pthread_mutex_init(&mutex, NULL);
	init();
}

void SZARKCommand::lock()
{
	pthread_mutex_lock(&mutex);
}

void SZARKCommand::unlock()
{
	pthread_mutex_unlock(&mutex);
}

#define COPY(x) (memcpy(&x, &source.x, sizeof(x)))
void SZARKCommand::copy(SZARKCommand source, bool nolock)
{
	if(!nolock) source.lock();

	// motors
	COPY(motors);
	// arm
	COPY(arm);
	// lights
	COPY(lights);
	// arm positions
	COPY(armPositions);
	armGoToPos = source.armGoToPos;
	// lcd
	memcpy(lcdText, source.lcdText, LCD_CHARACTERS + 1);
	// server
	performEmergencyStop = source.performEmergencyStop;
	performExit = source.performExit;

	if(!nolock) source.unlock();
}

void SZARKCommand::init()
{
	// motor & arm init for themselves

	// arm positions
	armGoToPos = false;
	memset(&armPositions, 0, sizeof(armPositions));
	// lcd
	memset(lcdText, 0, LCD_CHARACTERS + 1);
	// lights and stuff
	memset(&lights, 0, 	sizeof(lights));
	// server
	performEmergencyStop = false;
	performExit = false;
}

// SZARKStatus

SZARKStatus::SZARKStatus()
{
	pthread_mutex_init(&mutex, NULL);
	init();
}

void SZARKStatus::lock()
{
	pthread_mutex_lock(&mutex);
}

void SZARKStatus::unlock()
{
	pthread_mutex_unlock(&mutex);
}

void SZARKStatus::copy(SZARKStatus source)
{
	source.lock();

	COPY(battery);
	COPY(armPositions);
	COPY(armDirections);

	runlevel = source.runlevel;

	source.unlock();
}

void SZARKStatus::init()
{
	// battery
	battery.current = 0.0f;
	battery.voltage = 0.0f;

	// arm
	memset(&armPositions, 0, sizeof(armPositions));
	armDirections.shoulder = D_STOP;
	armDirections.elbow = D_STOP;
	armDirections.wrist = D_STOP;
	armDirections.gripper = D_STOP;	

	runlevel = STOPPED_SOFT;
}

