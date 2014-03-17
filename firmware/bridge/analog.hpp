/*
 * analog.h
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#ifndef ANALOG_H_
#define ANALOG_H_

namespace analog {
	void init();

	uint16_t getRawVoltage();
	uint16_t getRawCurrent();
}

#endif /* ANALOG_H_ */
