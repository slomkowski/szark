#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "global.h"
#include "extra_func.h"
#include "config.h"
#include "structures.h"

#include "bridge.h"
#include "networking.h"

#define END_COLOR "\033[0m"

void msgf(const char* frmt, ...)
{
#if LOGS_DISPLAY_TIME 
	char buff[32];
	time_t t;
	
	time(&t);
	strftime(buff,32,"[%H:%M:%S] ",localtime(&t)); // hours, minutes, seconds
	
	printf("%s", buff);
#endif
	printf(MESG_COLOR);

	va_list ap;
	va_start(ap, frmt);
	vprintf(frmt, ap);
	va_end(ap);

	printf(END_COLOR "\n");
	fflush(stdout);
}

// important message
void imsgf(const char* frmt, ...)
{
#if LOGS_DISPLAY_TIME 
	char buff[32];
	time_t t;
	
	time(&t);
	strftime(buff,32,"[%H:%M:%S] ",localtime(&t)); // hours, minutes, seconds
	
	printf("%s", buff);
#endif
	printf(IMPORTANT_MESG_COLOR);

	va_list ap;
	va_start(ap, frmt);
	vprintf(frmt, ap);
	va_end(ap);

	printf(END_COLOR "\n");
	fflush(stdout);
}

void errf(const char* frmt, ...)
{
#if LOGS_DISPLAY_TIME 
	char buff[32];
	time_t t;
	
	time(&t);
	strftime(buff,32,"[%H:%M:%S] ",localtime(&t));
	
	fprintf(stderr, "%s", buff);
#endif
	fprintf(stderr, ERR_COLOR);

	va_list ap;
	va_start(ap, frmt);
	vfprintf(stderr, frmt,ap);
	va_end(ap);

	fprintf(stderr, END_COLOR "\n");
	fflush(stdout);
}

void nexit(int status)
{
	b_shutdown();	
	networking_shutdown();
	sleep(1);
	imsgf("Exiting with status %d.\n", status);
	exit(status);
}

const char *direction_to_text(DIRECTION d)
{
	static const char *output[] = { "forward", "backward", "stopped" };
	switch(d)
	{
		case D_FORWARD: 	return output[0];	
		case D_BACKWARD:	return output[1];	 
		case D_STOP:		return output[2];	 
		default:
#if DEBUG
			errf("direction_to_text: Warning! Character \'%d\' is the wrong direction!", d);
#endif
			return output[2];
	};
}


void reboot()
{
					imsgf("Rebooting the machine.");
#if DUMMY_SHUTDOWN_REBOOT
					imsgf("DUMMY REBOOT PERFORMED: " MACHINE_REBOOT_COMMAND);
#else
					system(MACHINE_REBOOT_COMMAND);
#endif
}

void shutdown_machine()
{
					imsgf("Shutting down the machine.");
#if DUMMY_SHUTDOWN_REBOOT
					imsgf("DUMMY SHUTDOWN PERFORMED: " MACHINE_SHUTDOWN_COMMAND);
#else
					system(MACHINE_SHUTDOWN_COMMAND);
#endif
}
