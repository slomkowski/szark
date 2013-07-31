#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "global.h"
#include "structures.h"
#include "extra_func.h"

#include "bridge.h"
#include "networking.h"
#include "watchdog.h"

SZARKCommand main_command;
SZARKStatus  main_status;

void signal_catcher(int signal);

int main(int argc, char *argv[])
{
	pthread_t networking_thread, b_data_sender_thread, b_data_listener_thread, watchdog_thread;

	signal(SIGINT, signal_catcher);

	imsgf("SZARK server v. " VERSION ". Copyright 2011, 2012 by Michal Slomkowski.");
	msgf("You can redistribute this software under the terms of the General Public License.");

#if UART_ENABLE 
	char *port = UART_DEFAULT_DEVICE;
	for(int i = 0; i < argc - 1; i++)
	{
		if(!strcmp(argv[i], "-p")) port = argv[i + 1];
		break;
	}
	// setup bridge control
	if(b_setup(port, UART_BAUDRATE) == ERROR)
	{
		errf("Cannot initialize serial port, exiting.");
		nexit(EXIT_FAILURE);
	}
	if(pthread_create( &b_data_listener_thread, NULL, b_data_listener_function, NULL))
	{
		errf("pthread: unable to create a thread b_data_listener_function.");
		nexit(EXIT_FAILURE);
	}
	if(pthread_create( &b_data_sender_thread, NULL, b_data_sender_function, NULL))
	{
		errf("pthread: unable to create a thread b_data_sender_function.");
		nexit(EXIT_FAILURE);
	}
#else
#warning UART is disabled.
#endif

#if WATCHDOG_ENABLE
	// enabling watchdog
	if(pthread_create( &watchdog_thread, NULL, watchdog_function, NULL))
	{
		errf("pthread: unable to create a thread networking_function.");
		nexit(EXIT_FAILURE);
	}
#else
#warning Watchdog is disabled.
#endif

	// starting network support
	
	if(networking_setup(NETWORKING_PORT) == ERROR)
	{
		errf("Cannot initialize network, exiting.");
		nexit(EXIT_FAILURE);
	}
	if(pthread_create( &networking_thread, NULL, networking_function, NULL))
	{
		errf("pthread: unable to create a thread networking_function.");
		abort();
	}

#if UART_ENABLE 
	pthread_join(b_data_sender_thread, NULL);
#else 
	pthread_join(networking_thread, NULL);
#endif

	nexit(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}

void signal_catcher(int signal)
{
	if(signal == SIGINT)
	{
		imsgf("Catched SIGINT, exiting.");
		nexit(EXIT_SUCCESS);
	}
}

