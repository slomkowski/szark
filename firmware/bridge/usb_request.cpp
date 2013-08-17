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

#include "usb_request.h"
#include "lcd.h"
#include "analog.h"
#include "buttons.h"
#include "menu.h"
#include "delay.h"
#include "killswitch.h"
#include "expander.h"

#include "usb-commands.h"

struct Buffer {
	uint8_t data[64];
	uint8_t currentPosition;
	uint8_t length;

	void push(void *data, uint8_t length);

	void init();
};

void Buffer::push(void *data, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		this->data[currentPosition++] = *((uint8_t*) data + i);
	}
	this->length += length;
}

void Buffer::init() {
	currentPosition = 0;
	length = 0;
}

static Buffer inBuff, outBuff;

static volatile bool newCommandAvailable = false;

using namespace usb;

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *) data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_VENDOR) {
		return 0;
	}

	usbMsgPtr = uint16_t(&outBuff.data);
	usbMsgLen_t len;

	switch (static_cast<USBCommands::USBRequest>(rq->bRequest)) {
	case USBCommands::USB_READ:
		len = outBuff.length;
		if (len > rq->wLength.word) {
			len = rq->wLength.word;
		}
		return len;
	case USBCommands::USB_WRITE:
		inBuff.currentPosition = 0;
		inBuff.length = rq->wLength.word;
		return USB_NO_MSG ;
	};

	return 0;
}

uint8_t usbFunctionWrite(uint8_t *data, uint8_t len) {
	if (len > inBuff.length - inBuff.currentPosition) {
		len = inBuff.length - inBuff.currentPosition;
	}

	for (uint8_t i = 0; i < len; i++) {
		inBuff.data[inBuff.currentPosition++] = data[i];
	}

	if (inBuff.currentPosition >= inBuff.length - 1) {
		newCommandAvailable = true;
		return 1;
	} else {
		return 0;
	}
}

void usb::executeCommandsFromUSB() {
	if (not newCommandAvailable) {
		return;
	}

	inBuff.currentPosition = 0;

	outBuff.init();

	while (inBuff.currentPosition < inBuff.length - 1) {
		inBuff.currentPosition++;
		switch (static_cast<USBCommands::Request>(inBuff.data[inBuff.currentPosition - 1])) {
		case USBCommands::BRIDGE_GET_STATE: {
			USBCommands::bridge::State bState;
			bState.rawCurrent = analog::getRawCurrent();
			bState.rawVoltage = analog::getRawVoltage();

			auto buttons = buttons::getButtonsState(false);
			bState.buttonDown = buttons->down;
			bState.buttonUp = buttons->up;
			bState.buttonEnter = buttons->enter;

			bState.killSwitch = killswitch::isActive() ? USBCommands::bridge::ACTIVE : USBCommands::bridge::INACTIVE;
			outBuff.push(&bState, sizeof(bState));
		}
			break;
		case USBCommands::BRIDGE_SET_KILLSWITCH:
			if (inBuff.data[inBuff.currentPosition] == USBCommands::bridge::INACTIVE) {
				killswitch::setActive(false);
			} else {
				killswitch::setActive(true);
			}
			inBuff.currentPosition++;
			break;
		case USBCommands::EXPANDER_GET: {
			uint8_t val = expander::getValue();
			outBuff.push(&val, 1);
		}
			break;
		case USBCommands::EXPANDER_SET:
			expander::setValue(inBuff.data[inBuff.currentPosition]);
			inBuff.currentPosition++;
			break;
		case USBCommands::ARM_DRIVER_GET:
			// TODO implement arm_driver_get
			break;
		case USBCommands::MOTOR_DRIVER_GET:
			// TODO implement arm_driver_set
			break;
		case USBCommands::ARM_DRIVER_SET:
// TODO implement arm driver set
			break;
		case USBCommands::MOTOR_DRIVER_SET:
// TODO implement motor driver set
			break;
		case USBCommands::BRIDGE_LCD_SET:
			lcd::clrscr();
			lcd::puts(reinterpret_cast<char*>(&(inBuff.data[inBuff.currentPosition + 1])),
				inBuff.data[inBuff.currentPosition]);
			inBuff.currentPosition += inBuff.data[inBuff.currentPosition] + 1;
			break;
		};
	}

	newCommandAvailable = false;
}

