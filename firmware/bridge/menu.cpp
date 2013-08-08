/*
 * menu.cpp
 *
 *  Created on: 07-08-2013
 *      Author: michal
 */

#include "global.h"

#include <avr/pgmspace.h>
#include <stdlib.h>

#include "lcd.h"
#include "arm_driver.h"
#include "motor_driver.h"
#include "expander.h"
#include "buttons.h"
#include "analog.h"
#include "menu.h"
#include "killswitch.h"
#include "MenuClass.h"

using namespace menu;

namespace arm {
	const char CALIBRATE[] PROGMEM = "CALIBRATE";
	const char SHOULDER[] PROGMEM = "SHOULDER";
	const char ELBOW[] PROGMEM = "ELBOW";
	const char WRIST[] PROGMEM = "WRIST";
}

static MenuItem armMenuItems[] = { { "CALIBRATE", NULL }, { "SHOULDER", NULL }, { "ELBOW", NULL }, { "WRIST", NULL }, {
	"GRIPPER", NULL } };

static MenuItem motorMenuItems[] = { { "LEFT WHEEL", NULL }, { "RIGHT WHEEL", NULL }, { "BOTH WHEELS", NULL } };

static MenuClass armMenu("Arm driver:", 5, armMenuItems);
static MenuClass motorMenu("Motor driver:", 3, motorMenuItems);
static MenuClass expanderMenu("I2C expander:", 0);

static MenuItem mainMenuItems[] = { { "ARM DRIVER", &armMenu }, { "MOTOR DRIVER", &motorMenu }, { "I2C EXPANDER",
	&expanderMenu } };

static MenuClass mainMenu(NULL, 3, mainMenuItems);

const uint16_t BATTERY_VOLTAGE_FACTOR = 71;

static void batteryDisplay() {
	static char text[] = "Batt: xx.xV\n";

	uint8_t index;
	uint16_t volt = (analog::getRawVoltage() / 4) * BATTERY_VOLTAGE_FACTOR / 10;

	if (volt % 10 >= 5) {
		volt = volt / 10 + 1;
	} else {
		volt /= 10;
	}

	// aka itoa
	for (index = 9; index >= 6; index--) {
		if (index != 8) {
			text[index] = volt % 10 + '0';
			volt /= 10;
		}
	}

	lcd::puts(text);
}

static void expanderSubMenuFunction(uint8_t currentPosition, buttons::Buttons *buttonsState) {
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

static void motorSubMenuFunction(uint8_t currentPosition, buttons::Buttons *buttonsState) {
	motor::setSpeed(motor::LEFT, 3);
	motor::setSpeed(motor::RIGHT, 3);

	auto direction = motor::STOP;
	if (buttonsState->up) {
		direction = motor::FORWARD;
	} else if (buttonsState->down) {
		direction = motor::BACKWARD;
	}
	if (buttonsState->enter) {
		direction = motor::STOP;
	}

	switch (currentPosition) {
	case 0:
		lcd_putsP("MOTOR: left");
		motor::setDirection(motor::LEFT, direction);
		break;
	case 1:
		lcd_putsP("MOTOR: right");
		motor::setDirection(motor::RIGHT, direction);
		break;
	case 2:
		lcd_putsP("MOTOR: both");
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

void menu::init() {
	armMenu.setParent(&mainMenu);
	motorMenu.setParent(&mainMenu);
	expanderMenu.setParent(&mainMenu);

	mainMenu.setHeaderFunction(batteryDisplay);

	expanderMenu.setSubMenuFunction(expanderSubMenuFunction);
	motorMenu.setSubMenuFunction(motorSubMenuFunction);
}

void menu::poll() {
	mainMenu.process();
}
