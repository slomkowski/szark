/*
 * usb_request.h
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#ifndef USB_REQUEST_H_
#define USB_REQUEST_H_

namespace usb {
	void executeCommandsFromUSB();

	bool wasKillSwitchDisabled();
}

#endif /* USB_REQUEST_H_ */
