/*
 * global.h
 *
 *  Created on: 05-08-2013
 *      Author: michal
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

/*
 * CPU clock frequency. Set to 16 MHz.
 */
#define F_CPU 16000000UL

/*
 * Useful macros for port name resolving.
 */
#define PORT(x) XPORT(x)
#define XPORT(x) (PORT##x)

#define PIN(x) XPIN(x)
#define XPIN(x) (PIN##x)

#define DDR(x) XDDR(x)
#define XDDR(x) (DDR##x)

#endif /* GLOBAL_H_ */
