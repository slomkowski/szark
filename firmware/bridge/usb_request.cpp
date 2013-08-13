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

static uint8_t buffer[35];

static uint8_t currentPosition, bytesRemaining;

static volatile USBCommands::Request actualRequest;
static volatile bool newCommandAvailable = false;

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *) data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_VENDOR) {
		return 0;
	}

	usbMsgPtr = uint16_t(&buffer);

	switch (static_cast<USBCommands::Request>(rq->bRequest)) {
	case USBCommands::BRIDGE_GET_STATE: {
		auto bState = reinterpret_cast<USBCommands::bridge::State*>(&buffer);
		bState->rawCurrent = analog::getRawCurrent();
		bState->rawVoltage = analog::getRawVoltage();

		auto buttons = buttons::getButtonsState(false);
		bState->buttonDown = buttons->down;
		bState->buttonUp = buttons->up;
		bState->buttonEnter = buttons->enter;

		bState->killSwitch = killswitch::isActive() ? USBCommands::bridge::ACTIVE : USBCommands::bridge::INACTIVE;

		return sizeof(USBCommands::bridge::State);
	}
	case USBCommands::BRIDGE_SET_KILLSWITCH:
		if (rq->wValue.bytes[0] == USBCommands::bridge::INACTIVE) {
			killswitch::setActive(false);
		} else {
			killswitch::setActive(true);
		}
		return 0;
	case USBCommands::EXPANDER_GET:
		buffer[0] = expander::getValue();
		return 1;
	case USBCommands::EXPANDER_SET:
		expander::setValue(rq->wValue.bytes[0]);
		return 0;
	case USBCommands::ARM_DRIVER_GET:
		// TODO implement arm_driver_get
		return 0;
	case USBCommands::MOTOR_DRIVER_GET:
		// TODO implement arm_driver_set
		return 0;
	case USBCommands::BRIDGE_LCD_SET:
	case USBCommands::ARM_DRIVER_SET:
	case USBCommands::MOTOR_DRIVER_SET:
		actualRequest = static_cast<USBCommands::Request>(rq->bRequest);
		currentPosition = 0;
		bytesRemaining = rq->wLength.word;
		return USB_NO_MSG ;
	};
	return 0;
}

uint8_t usbFunctionWrite(uint8_t *data, uint8_t len) {
	if (len > bytesRemaining) {
		len = bytesRemaining;
	}
	bytesRemaining -= len;

	for (uint8_t i = 0; i < len; i++) {
		buffer[currentPosition++] = data[i];
	}

	if (bytesRemaining == 0) {
		buffer[currentPosition++] = 0;
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

	switch (actualRequest) {
	case USBCommands::BRIDGE_LCD_SET:
		lcd::clrscr();
		lcd::puts(reinterpret_cast<char*>(buffer));
		break;
	case USBCommands::ARM_DRIVER_SET:
		// TODO implement arm driver set
		break;
	case USBCommands::MOTOR_DRIVER_SET:
		// TODO implement motor driver set
		break;
	};

	newCommandAvailable = false;
}

