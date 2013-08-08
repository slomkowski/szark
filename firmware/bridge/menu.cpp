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

static Menu armMenu("Arm driver:", 5, armMenuItems);
static Menu motorMenu("Motor driver:", 3, motorMenuItems);
static Menu expanderMenu("I2C expander:", 0);

static MenuItem mainMenuItems[] = { { "ARM DRIVER", &armMenu }, { "MOTOR DRIVER", &motorMenu }, { "I2C EXPANDER",
	&expanderMenu } };

static Menu mainMenu(NULL, 3, mainMenuItems);

static Menu* actualMenu;

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
	motor_driver::setSpeed(motor_driver::LEFT, 3);
	motor_driver::setSpeed(motor_driver::RIGHT, 3);

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
		motor_driver::setDirection(motor_driver::LEFT, direction);
		break;
	case 1:
		lcd_putsP("MOTOR: right");
		motor_driver::setDirection(motor_driver::RIGHT, direction);
		break;
	case 2:
		lcd_putsP("MOTOR: both");
		motor_driver::setDirection(motor_driver::LEFT, direction);
		motor_driver::setDirection(motor_driver::RIGHT, direction);
		break;
	}

	static char speedText[] = "\nL: xxx, R: xxx";
	auto motorSpeed = motor_driver::getSpeed(motor_driver::LEFT);
	for (uint8_t i = 6; i > 3; i--) {
		speedText[i] = (motorSpeed % 10 + '0');
		motorSpeed /= 10;
	}
	motorSpeed = motor_driver::getSpeed(motor_driver::RIGHT);
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

	actualMenu = &mainMenu;
}

void menu::poll() {
	actualMenu->process();
}

void Menu::process() {
	lcd::clrscr();

	auto *buttons = buttons::getButtonsState();

	if (inSubMenuFunction or itemsLength == 0) {
		if (subMenuFunction == NULL) {
			lcd::clrscr();
			lcd::putsp(PSTR("No submenu function nor menu items!"));
		} else {
			if (not inSubMenuFunction) {
				killswitch::setActive(false);
			}
			inSubMenuFunction = true;
			(*subMenuFunction)(this->currentPosition, buttons);
		}
	} else {
		if (this->title != NULL) {
			lcd::puts(title);
		} else if (this->headerFunction != NULL) {
			(*headerFunction)();
		}

		if (itemsLength > 0) {
			lcd::putsp(0, 1, PSTR("< "));
			if (currentPosition == itemsLength) {
				lcd::putsp(PSTR("EXIT"));
			} else {
				lcd::puts(menuItems[currentPosition].name);
			}
			lcd::putsp(PSTR(" >"));

			if (buttons->up) {
				if ((parent == NULL and currentPosition == itemsLength - 1) or currentPosition == itemsLength) {
					currentPosition = 0;
				} else {
					currentPosition++;
				}
			} else if (buttons->down) {
				if (currentPosition == 0) {
					if (parent == NULL) {
						currentPosition = itemsLength - 1;
					} else {
						currentPosition = itemsLength;
					}
				} else {
					currentPosition--;
				}
			}
		}
	}

	if (buttons->enter) {
		if (currentPosition == itemsLength) {
			actualMenu = parent;
		} else {
			if (subMenuFunction != NULL) {
				if (not inSubMenuFunction) {
					inSubMenuFunction = true;
					killswitch::setActive(false);
					(*subMenuFunction)(this->currentPosition, buttons);
				} else {
					inSubMenuFunction = false;
					killswitch::setActive(true);
					if (itemsLength == 0) {
						actualMenu = parent;
					}
				}
			} else if (menuItems[currentPosition].subMenu != NULL) {
				actualMenu = menuItems[currentPosition].subMenu;
				actualMenu->currentPosition = 0;
			} else {
				lcd::clrscr();
				lcd::putsp(PSTR("No function nor submenu!"));
			}
		}
	}
}

Menu::Menu(const char* title, uint8_t itemsLength, MenuItem menuItems[], Menu *parent) {
	this->title = title;
	this->menuItems = menuItems;
	this->itemsLength = itemsLength;
	this->parent = parent;

	this->currentPosition = 0;
	this->headerFunction = NULL;
	this->subMenuFunction = NULL;
	this->inSubMenuFunction = false;
}
