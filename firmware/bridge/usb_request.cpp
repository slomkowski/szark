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
#include "motor_driver.h"
#include "arm_driver.h"

#include "usb-commands.h"

struct Buffer {
	uint8_t data[90] = { 0xff };
	uint8_t currentPosition;
	uint8_t length;

	void push(void *data, uint8_t length);

	void init();
};

void Buffer::push(void *data, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		this->data[currentPosition] = *((uint8_t*) data + i);
		currentPosition++;
	}
	this->length += length;
}

void Buffer::init() {
	currentPosition = 0;
	length = 0;
}

static Buffer inBuff, outBuff;

static bool killSwitchDisabled = false;

static volatile bool newCommandAvailable = false;
static volatile bool responseReady = false;

using namespace usb;

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *) data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_VENDOR) {
		return 0;
	}

	usbMsgLen_t len;

	switch (static_cast<USBCommands::USBRequest>(rq->bRequest)) {
	case USBCommands::USB_READ:
		usbMsgPtr = uint16_t(&outBuff.data);
		len = outBuff.length;
		if (len > rq->wLength.word) {
			len = rq->wLength.word;
		}
		return len;
	case USBCommands::USB_WRITE:
		inBuff.currentPosition = 0;
		inBuff.length = rq->wLength.word;
		responseReady = false;
		return USB_NO_MSG ;
	case USBCommands::IS_RESPONSE_READY:
		usbMsgPtr = uint16_t(&responseReady);
		return 1;
	};

	return 0;
}

uint8_t usbFunctionWrite(uint8_t *data, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		inBuff.data[inBuff.currentPosition] = data[i];
		inBuff.currentPosition++;
	}

	if (len != 8) {
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

	for (inBuff.currentPosition = 1; inBuff.currentPosition <= inBuff.length; inBuff.currentPosition++) {
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
				killSwitchDisabled = true;
				killswitch::setActive(false);
			} else {
				killSwitchDisabled = false;
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
		case USBCommands::ARM_DRIVER_GET_GENERAL_STATE: {
			USBCommands::arm::GeneralState general;
			general.isCalibrated = 66; //arm::isCalibrated();
			general.mode = (arm::Mode) 66; //arm::getMode();
			outBuff.push(&general, sizeof(USBCommands::arm::GeneralState));
		}
			break;
		case USBCommands::ARM_DRIVER_GET: {
			USBCommands::arm::JointState joint;
			arm::Motor m = static_cast<arm::Motor>(inBuff.data[inBuff.currentPosition]);
			joint.motor = m;
			joint.direction = arm::getDirection(m);
			joint.speed = arm::getSpeed(m);
			joint.position = arm::getPosition(m);
			joint.setPosition = false;
			outBuff.push(&joint, sizeof(USBCommands::arm::JointState));
			inBuff.currentPosition++;
		}
			break;
		case USBCommands::MOTOR_DRIVER_GET: {
			USBCommands::motor::SpecificMotorState mState;
			motor::Motor m = static_cast<motor::Motor>(inBuff.data[inBuff.currentPosition]);
			mState.motor = m;
			mState.speed = motor::getSpeed(m);
			//mState.direction = motor::getDirection(m);
			outBuff.push(&mState, sizeof(USBCommands::motor::SpecificMotorState));
			inBuff.currentPosition++;
		}
			break;
		case USBCommands::ARM_DRIVER_SET:
			if (inBuff.data[inBuff.currentPosition] == USBCommands::arm::BRAKE) {
				arm::brake();
				inBuff.currentPosition++;
			} else if (inBuff.data[inBuff.currentPosition] == USBCommands::arm::CALIBRATE) {
				arm::calibrate();
				inBuff.currentPosition++;
			} else {
				auto joint = reinterpret_cast<USBCommands::arm::JointState*>(&inBuff.data[inBuff.currentPosition]);
				arm::setSpeed(static_cast<arm::Motor>(joint->motor), joint->speed);
				if (joint->setPosition) {
					arm::setPosition(static_cast<arm::Motor>(joint->motor), joint->position);
				} else {
					arm::setDirection(static_cast<arm::Motor>(joint->motor),
						static_cast<arm::Direction>(joint->direction));
				}
				inBuff.currentPosition += sizeof(USBCommands::arm::JointState);
			}
			break;
		case USBCommands::MOTOR_DRIVER_SET: {
			auto m = reinterpret_cast<USBCommands::motor::SpecificMotorState*>(&inBuff.data[inBuff.currentPosition]);
			motor::setSpeed(static_cast<motor::Motor>(m->motor), m->speed);
			motor::setDirection(static_cast<motor::Motor>(m->motor), static_cast<motor::Direction>(m->direction));
			inBuff.currentPosition += sizeof(USBCommands::motor::SpecificMotorState);
		}
			break;
		case USBCommands::BRIDGE_LCD_SET: {
			uint8_t length = inBuff.data[inBuff.currentPosition];
			char* text = reinterpret_cast<char*>(&(inBuff.data[inBuff.currentPosition + 1]));

			lcd::clrscr();
			lcd::puts(text, length);
			inBuff.currentPosition += length + 1;
		}
			break;
		};

		usbPoll();
	}

	responseReady = true;
	newCommandAvailable = false;
}

bool usb::wasKillSwitchDisabled() {
	return killSwitchDisabled;
}
