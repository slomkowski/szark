#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

void *watchdog_function(void *arg);

void watchdog_reset();
void watchdog_disable();

#endif

