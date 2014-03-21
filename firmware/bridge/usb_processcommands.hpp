/*
 * usb_processcommands.hpp
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#ifndef USB_PROCESSCOMMANDS_H_
#define USB_PROCESSCOMMANDS_H_

namespace usb {
	static const uint8_t BUFFER_SIZE = 128;

	class Buffer {
	public:
		uint8_t data[BUFFER_SIZE + 1] = { 0xff };
		uint8_t length;
		uint8_t currentPosition;

		void push(void *data, uint8_t length);

		void init();
	};

	extern Buffer inBuff, outBuff;

	void executeCommandsFromUSB();

	bool wasKillSwitchDisabled();
}

#endif /* USB_PROCESSCOMMANDS_HPP_ */
