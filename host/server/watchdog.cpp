#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "global.h"
#include "watchdog.h"
#include "extra_func.h"

static volatile bool enabled = false;

#define STEPS 20
#define STEP_INTERVAL (WATCHDOG_INTERVAL / STEPS)

static volatile int counter = WATCHDOG_INTERVAL;
static pthread_mutex_t watchdog_counter_mutex  = PTHREAD_MUTEX_INITIALIZER;

void *watchdog_function(void *arg)
{
	while(1)
	{	
		pthread_mutex_lock(&watchdog_counter_mutex);
		if(counter >= 0)
		{
			if(enabled) counter -= STEP_INTERVAL;
		}
		else // perform reset
		{
			imsgf("watchdog: Watchdog performed a reset!");
			//
			main_command.init();
			main_command.lock();
			bzero(main_command.lcdText, LCD_CHARACTERS);
			strcpy(main_command.lcdText, "Wifi signal lost, stopped.");
			main_command.unlock();
			
			// reset watchdog
			enabled = false;
			counter = WATCHDOG_INTERVAL;
		}
		pthread_mutex_unlock(&watchdog_counter_mutex);

		usleep(STEP_INTERVAL * 1000);
	}

	return NULL;
}

void watchdog_reset()
{
	pthread_mutex_lock(&watchdog_counter_mutex);

	counter = WATCHDOG_INTERVAL;
	enabled = true;

	pthread_mutex_unlock(&watchdog_counter_mutex);
}

void watchdog_disable()
{
	pthread_mutex_lock(&watchdog_counter_mutex);

	counter = WATCHDOG_INTERVAL;
	enabled = false;

	pthread_mutex_unlock(&watchdog_counter_mutex);
}

