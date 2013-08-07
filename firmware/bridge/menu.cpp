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

void menu::init() {
	armMenu.setParent(&mainMenu);
	motorMenu.setParent(&mainMenu);
	expanderMenu.setParent(&mainMenu);

	mainMenu.setHeaderFunction(batteryDisplay);

	actualMenu = &mainMenu;
}

void menu::poll() {
	actualMenu->process();
}

void Menu::process() {
	lcd::clrscr();

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

		buttons::Buttons *buttons = buttons::getButtonsState();

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
		} else if (buttons->enter) {
			if (currentPosition == itemsLength) {
				actualMenu = parent;
			} else {
				if (subMenuFunction != NULL) {
					(*subMenuFunction)(this->currentPosition);
				} else if (menuItems[currentPosition].subMenu != NULL) {
					actualMenu = menuItems[currentPosition].subMenu;
				} else {
					lcd::clrscr();
					lcd::putsp(PSTR("No function nor submenu!"));
				}
			}
		}
	}
}

Menu::Menu(const char* title, uint8_t itemsLength, MenuItem menuItems[], Menu *parent) {
	this->title = title;
	this->menuItems = menuItems;
	this->itemsLength = itemsLength;
	this->parent = parent;
}
