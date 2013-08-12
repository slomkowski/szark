/*
 * usb_request.cpp
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#include "global.h"
#include <inttypes.h>

extern "C" {
#include "usbdrv.h"
}

#include "lcd.h"
#include "analog.h"
#include "buttons.h"
#include "menu.h"
#include "delay.h"
#include "killswitch.h"

#include "usb-commands.h"

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *) data;

	static uint8_t buffer[16];

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR) {
		switch (rq->bRequest) {
		case USBCommands::BRIDGE_GET_STATE:
			auto bState = reinterpret_cast<USBCommands::bridge::State*>(&buffer);
			bState->rawCurrent = analog::getRawCurrent();
			bState->rawVoltage = analog::getRawVoltage();

			usbMsgPtr = uint16_t(&buffer);
			return sizeof(USBCommands::bridge::State);
		};
	} else {
		/* calss requests USBRQ_HID_GET_REPORT and USBRQ_HID_SET_REPORT are
		 * not implemented since we never call them. The operating system
		 * won't call them either because our descriptor defines no meaning.
		 */
	}
	return 0; /* default for not implemented requests: return no data back to host */
}

