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

void menu::init() {
	armMenu.setParent(&mainMenu);
	motorMenu.setParent(&mainMenu);
	expanderMenu.setParent(&mainMenu);

	actualMenu = &mainMenu;
}

void menu::poll() {
	actualMenu->process();
}

Menu::Menu(const char* title, uint8_t itemsLength, MenuItem menuItems[], Menu *parent) {
	this->currentPosition = 0;
	this->title = title;
	this->menuItems = menuItems;
	this->itemsLength = itemsLength;
	this->parent = parent;
}

void Menu::process() {
	lcd::clrscr();

	if (this->title != NULL) {
		lcd::puts(title);
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
		}

		if (buttons->down) {
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

		if (buttons->enter) {
			if (currentPosition == itemsLength) {
				actualMenu = parent;
			} else {
				actualMenu = menuItems[currentPosition].subMenu;
			}
		}
	}
}
