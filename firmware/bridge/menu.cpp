/*
 * menu.cpp
 *
 *  Created on: 07-08-2013
 *      Author: michal
 */

#include "global.hpp"

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.hpp"
#include "arm_driver.hpp"
#include "motor_driver.hpp"
#include "expander.hpp"
#include "buttons.hpp"
#include "analog.hpp"
#include "menu.hpp"
#include "killswitch.hpp"
#include "i2c.hpp"
#include "MenuClass.hpp"

using namespace menu;

/*
 * Hardware settings
 */
const double BATTERY_VOLTAGE_FACTOR = .01981590757978723404;

const uint8_t MOTOR_SPEED = 3;

const uint8_t ARM_SPEED = 5;
const uint8_t ARM_SHOULDER_SPEED = ARM_SPEED;
const uint8_t ARM_ELBOW_SPEED = ARM_SPEED;
const uint8_t ARM_WRIST_SPEED = ARM_SPEED;
const uint8_t ARM_GRIPPER_SPEED = ARM_SPEED;

static MenuItem armMenuItems[] = { { "CALIBRATE", nullptr }, { "SHOULDER", nullptr }, { "ELBOW", nullptr }, { "WRIST",
	nullptr }, { "GRIPPER", nullptr } };

static MenuItem motorMenuItems[] = { { "LEFT WHEEL", nullptr }, { "RIGHT WHEEL", nullptr }, { "BOTH WHEELS", nullptr } };

static MenuClass armMenu("Arm driver:", 5, armMenuItems);
static MenuClass motorMenu("Motor driver:", 3, motorMenuItems);
static MenuClass expanderMenu("I2C expander:", 0);

static MenuItem mainMenuItems[] = { { "ARM DRIVER", &armMenu }, { "MOTOR DRIVER", &motorMenu }, { "I2C EXPANDER",
	&expanderMenu } };

static MenuClass mainMenu(nullptr, 3, mainMenuItems);

static MenuClass *actualMenu;

static bool isInMenu = false;

static void batteryDisplayHeaderFunction() {
	static char text[] = "Batt: xx.xV\n";

	uint8_t index;
	uint16_t volt = analog::getRawVoltage() * 100.0 * BATTERY_VOLTAGE_FACTOR;

	if (volt % 10 >= 5) {
		volt = volt / 10 + 1;
	} else {
		volt /= 10;
	}

	for (index = 9; index >= 6; index--) {
		if (index != 8) {
			text[index] = volt % 10 + '0';
			volt /= 10;
		}
	}

	lcd::puts(text);
}

static void expanderSubMenuFunction(bool isFirstCall, uint8_t, buttons::Buttons *buttonsState) {
	if (not isFirstCall and i2c::getLastCommandStatus() != i2c::OK) {
		lcd::putsp(PSTR("Could not connect to expander!"));
		return;
	}

	static uint8_t currentDevice = 8;
	lcd::putsp(PSTR("EXPANDER:\nDevice: "));

	if (buttonsState->enter) {
		currentDevice = 8;
		expander::setValue(0);
		return;
	}

	if (buttonsState->up) {
		currentDevice++;
		if (currentDevice > 8) {
			currentDevice = 0;
		}
	}
	if (buttonsState->down) {
		if (currentDevice == 0) {
			currentDevice = 8;
		} else {
			currentDevice--;
		}
	}

	if (currentDevice == 8) {
		lcd::putsp(PSTR("OFF"));
	} else {
		lcd::putc('0' + currentDevice);
	}

	expander::setValue(1 << currentDevice);
}

static void motorSubMenuFunction(bool isFirstCall, uint8_t currentPosition, buttons::Buttons *buttonsState) {
	if (not isFirstCall and i2c::getLastCommandStatus() != i2c::OK) {
		lcd::putsp(PSTR("Motor driver not visible!"));
		return;
	}

	if (isFirstCall) {
		motor::setSpeed(motor::LEFT, MOTOR_SPEED);
		motor::setSpeed(motor::RIGHT, MOTOR_SPEED);
	}

	auto direction = motor::STOP;
	if (buttonsState->up) {
		direction = motor::FORWARD;
	} else if (buttonsState->down) {
		direction = motor::BACKWARD;
	}
	if (buttonsState->enter) {
		direction = motor::STOP;
	}

	lcd_putsP("MOTOR: ");
	switch (currentPosition) {
	case 0:
		lcd_putsP("left");
		motor::setDirection(motor::LEFT, direction);
		break;
	case 1:
		lcd_putsP("right");
		motor::setDirection(motor::RIGHT, direction);
		break;
	case 2:
		lcd_putsP("both");
		motor::setDirection(motor::LEFT, direction);
		motor::setDirection(motor::RIGHT, direction);
		break;
	}

	static char speedText[] = "\nL: xxx, R: xxx";
	auto motorSpeed = motor::getSpeed(motor::LEFT);
	for (uint8_t i = 6; i > 3; i--) {
		speedText[i] = (motorSpeed % 10 + '0');
		motorSpeed /= 10;
	}
	motorSpeed = motor::getSpeed(motor::RIGHT);
	for (uint8_t i = 14; i > 11; i--) {
		speedText[i] = (motorSpeed % 10 + '0');
		motorSpeed /= 10;
	}
	lcd::puts(speedText);
}

static void armSubMenuFunction(bool isFirstCall, uint8_t currentPosition, buttons::Buttons *buttonsState) {
	if (not isFirstCall and i2c::getLastCommandStatus() != i2c::OK) {
		lcd_putsP("Arm driver not visible!");
		return;
	}

	static bool calibrationFired;
	if (isFirstCall) {
		calibrationFired = false;
		arm::setSpeed(arm::SHOULDER, ARM_SHOULDER_SPEED);
		arm::setSpeed(arm::ELBOW, ARM_ELBOW_SPEED);
		arm::setSpeed(arm::WRIST, ARM_WRIST_SPEED);
		arm::setSpeed(arm::GRIPPER, ARM_GRIPPER_SPEED);
	}

	lcd::putsp(PSTR("ARM: "));

	auto direction = arm::STOP;
	auto motor = arm::SHOULDER;

	if (buttonsState->up) {
		direction = arm::FORWARD;
	} else if (buttonsState->down) {
		direction = arm::BACKWARD;
	}
	if (buttonsState->enter) {
		direction = arm::STOP;
	}

	switch (currentPosition) {
	case 0: // calibrate
		lcd_putsP("calibrate\n");
		if (not calibrationFired) {
			arm::calibrate();
			calibrationFired = true;
			return;
		}

		if (arm::isCalibrated()) {
			lcd_putsP("finished: YES");
		} else {
			lcd_putsP("finished: NO");
		}
		return;
	case 1: // shoulder
		lcd_putsP("shoulder\n");
		motor = arm::SHOULDER;
		break;
	case 2:
		lcd_putsP("elbow\n");
		motor = arm::ELBOW;
		break;
	case 3:
		lcd_putsP("wrist\n");
		motor = arm::WRIST;
		break;
	case 4:
		lcd_putsP("gripper\n");
		motor = arm::GRIPPER;
		break;
	}

	arm::setDirection(motor, direction);
	auto armPosition = arm::getPosition(motor);

	static char armPositionText[] = "Position: XXX"; // X fields are replaced by digits
	for (uint8_t i = 12; i > 9; i--) {
		armPositionText[i] = (armPosition % 10 + '0');
		armPosition /= 10;
	}
	lcd::puts(armPositionText);
}

void menu::init() {
	armMenu.setParent(&mainMenu);
	motorMenu.setParent(&mainMenu);
	expanderMenu.setParent(&mainMenu);

	mainMenu.setHeaderFunction(batteryDisplayHeaderFunction);

	expanderMenu.setSubMenuFunction(expanderSubMenuFunction);
	motorMenu.setSubMenuFunction(motorSubMenuFunction);
	armMenu.setSubMenuFunction(armSubMenuFunction);

	actualMenu = &mainMenu;
}

void menu::poll() {
	// this was done to limit stack size
	actualMenu = actualMenu->getActualMenu();

	if (isInMenu) {
		actualMenu->process();
	} else {
		auto btn = buttons::getButtonsState();

		if (btn->down or btn->up or btn->enter) {
			isInMenu = true;
		}
	}
}

void menu::goOutOfMenu() {
	isInMenu = false;
}

