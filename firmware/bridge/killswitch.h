/*
 * killswitch.h
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#ifndef KILLSWITCH_H_
#define KILLSWITCH_H_

namespace killswitch {
	void init();

	void setActive(bool active);

	bool isActive();

	bool isCausedByHardware();
}

#endif /* KILLSWITCH_H_ */
