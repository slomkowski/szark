#ifndef _MSG_FUNCTIONS_H_
#define _MSG_FUNCTIONS_H_

#include "structures.h"

void msgf(const char* frmt,...); // replaces standard printf
void errf(const char* frmt,...); // as above, but puts the data to stderr
void imsgf(const char* frmt, ...); // important message msgf

const char *direction_to_text(DIRECTION d);

void nexit(int status); // new exit()

void reboot();
void shutdown_machine();

#endif
