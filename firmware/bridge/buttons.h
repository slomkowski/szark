/*
 * button.h
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#ifndef BUTTON_H_
#define BUTTON_H_

namespace buttons {
	struct Buttons {
		bool up;
		bool down;
		bool enter;
	};

	void init();

	Buttons *getButtonsState(bool debounce = true);
}

#endif /* BUTTON_H_ */
