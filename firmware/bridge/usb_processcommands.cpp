/*
 * usb_request.cpp
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#include <inttypes.h>
#include <avr/wdt.h>

#include "global.hpp"
#include "usb_processcommands.hpp"
#include "lcd.hpp"
#include "analog.hpp"
#include "buttons.hpp"
#include "menu.hpp"
#include "delay.hpp"
#include "killswitch.hpp"
#include "expander.hpp"
#include "motor_driver.hpp"
#include "arm_driver.hpp"

#include "usb-commands.hpp"

using namespace usb;

void usb::Buffer::push(void *data, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		this->data[currentPosition] = *((uint8_t*) data + i);
		currentPosition++;
	}
	this->length += length;
}

void usb::Buffer::init() {
	currentPosition = 0;
	length = 0;
}

Buffer usb::InputBuff, usb::OutputBuff;

static volatile bool responseReady = false;

void usb::executeCommandsFromUSB() {
	InputBuff.currentPosition = 0;
	OutputBuff.init();

	for (InputBuff.currentPosition = 1; InputBuff.currentPosition <= InputBuff.length; InputBuff.currentPosition++) {
		switch (static_cast<USBCommands::Request>(InputBuff.data[InputBuff.currentPosition - 1])) {
		case USBCommands::BRIDGE_GET_STATE: {
			USBCommands::bridge::State bState;
			bState.rawCurrent = analog::getRawCurrent();
			bState.rawVoltage = analog::getRawVoltage();

			auto buttons = buttons::getButtonsState(false);
			bState.buttonDown = buttons->down;
			bState.buttonUp = buttons->up;
			bState.buttonEnter = buttons->enter;

			bState.killSwitch = killswitch::isActive() ? USBCommands::bridge::ACTIVE : USBCommands::bridge::INACTIVE;
			bState.killSwitchCausedByHardware = killswitch::isCausedByHardware();
			OutputBuff.push(&bState, sizeof(USBCommands::bridge::State));
		}
			break;
		case USBCommands::BRIDGE_SET_KILLSWITCH:
			if (InputBuff.data[InputBuff.currentPosition] == USBCommands::bridge::INACTIVE) {
				killswitch::setActive(false);
			} else {
				killswitch::setActive(true);
			}
			InputBuff.currentPosition++;
			break;
		case USBCommands::EXPANDER_GET: {
			uint8_t val = expander::getValue();
			OutputBuff.push(&val, 1);
		}
			break;
		case USBCommands::EXPANDER_SET:
			expander::setValue(InputBuff.data[InputBuff.currentPosition]);
			InputBuff.currentPosition++;
			break;
		case USBCommands::ARM_DRIVER_GET_GENERAL_STATE: {
			USBCommands::arm::GeneralState general;
			general.isCalibrated = 66; //arm::isCalibrated();
			general.mode = (arm::Mode) 66; //arm::getMode();
			OutputBuff.push(&general, sizeof(USBCommands::arm::GeneralState));
		}
			break;
		case USBCommands::ARM_DRIVER_GET: {
			USBCommands::arm::JointState joint;
			arm::Motor m = static_cast<arm::Motor>(InputBuff.data[InputBuff.currentPosition]);
			joint.motor = m;
			joint.direction = arm::getDirection(m);
			joint.speed = arm::getSpeed(m);
			joint.position = arm::getPosition(m);
			joint.setPosition = false;
			OutputBuff.push(&joint, sizeof(USBCommands::arm::JointState));
			InputBuff.currentPosition++;
		}
			break;
		case USBCommands::MOTOR_DRIVER_GET: {
			USBCommands::motor::SpecificMotorState mState;
			motor::Motor m = static_cast<motor::Motor>(InputBuff.data[InputBuff.currentPosition]);
			mState.motor = m;
			mState.speed = motor::getSpeed(m);
			//mState.direction = motor::getDirection(m);
			OutputBuff.push(&mState, sizeof(USBCommands::motor::SpecificMotorState));
			InputBuff.currentPosition++;
		}
			break;
		case USBCommands::ARM_DRIVER_SET:
			if (InputBuff.data[InputBuff.currentPosition] == USBCommands::arm::BRAKE) {
				arm::brake();
				InputBuff.currentPosition++;
			} else if (InputBuff.data[InputBuff.currentPosition] == USBCommands::arm::CALIBRATE) {
				arm::calibrate();
				InputBuff.currentPosition++;
			} else {
				auto joint = reinterpret_cast<USBCommands::arm::JointState*>(&InputBuff.data[InputBuff.currentPosition]);
				arm::setSpeed(static_cast<arm::Motor>(joint->motor), joint->speed);
				if (joint->setPosition) {
					arm::setPosition(static_cast<arm::Motor>(joint->motor), joint->position);
				} else {
					arm::setDirection(static_cast<arm::Motor>(joint->motor),
							static_cast<arm::Direction>(joint->direction));
				}
				InputBuff.currentPosition += sizeof(USBCommands::arm::JointState);
			}
			break;
		case USBCommands::MOTOR_DRIVER_SET: {
			auto m =
					reinterpret_cast<USBCommands::motor::SpecificMotorState*>(&InputBuff.data[InputBuff.currentPosition]);
			motor::setSpeed(static_cast<motor::Motor>(m->motor), m->speed);
			motor::setDirection(static_cast<motor::Motor>(m->motor), static_cast<motor::Direction>(m->direction));
			InputBuff.currentPosition += sizeof(USBCommands::motor::SpecificMotorState);
		}
			break;
		case USBCommands::BRIDGE_LCD_SET: {
			uint8_t length = InputBuff.data[InputBuff.currentPosition];
			char* text = reinterpret_cast<char*>(&(InputBuff.data[InputBuff.currentPosition + 1]));

			lcd::clrscr();
			lcd::puts(text, length);
			InputBuff.currentPosition += length + 1;
		}
			break;
		case USBCommands::MESSAGE_END:
			InputBuff.currentPosition = InputBuff.length + 1; // finish processing
			break;
		case USBCommands::BRIDGE_RESET_DEVICE:
			wdt_enable(WDTO_15MS);
			while (true) {
			}
			break;
		};
	}

	auto MESSAGE_END = USBCommands::MESSAGE_END;
	OutputBuff.push(&MESSAGE_END, 1);

	responseReady = true;
}
