/*
 * delay.hpp
 *
 *  Created on: 10-08-2013
 *      Author: michal
 */

#ifndef DELAY_H_
#define DELAY_H_

/**
 * Some delay functions used in various places. These functions guarantee to wait the time specified, but could
 * exceed it. They can't be used in time measurement.
 */
namespace delay {
	/**
	 * This function waits at least 100 us. Delay is based on assembly routine.
	 */
	void wait100us();

	/**
	 * This function guarantees to wait the specified amount of milliseconds, but can exceed this time due
	 * to some tasks performed.
	 * @param milliseconds
	 */
	void waitMs(uint16_t milliseconds);
}

#endif /* DELAY_HPP_ */
