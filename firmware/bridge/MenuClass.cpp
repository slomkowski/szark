/*
 * MenuClass.cpp
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#include "global.h"

#include <avr/pgmspace.h>
#include <stdlib.h>

#include "lcd.h"
#include "buttons.h"
#include "killswitch.h"
#include "MenuClass.h"

using namespace menu;

MenuClass *MenuClass::actualMenu;

MenuClass::MenuClass(const char* title, uint8_t itemsLength, MenuItem menuItems[], MenuClass *parent) {
	this->title = title;
	this->menuItems = menuItems;
	this->itemsLength = itemsLength;
	this->parent = parent;

	this->currentPosition = 0;
	this->headerFunction = NULL;
	this->subMenuFunction = NULL;
	this->inSubMenuFunction = false;
	this->actualMenu = this;
}

void MenuClass::process() {
	if (actualMenu != this) {
		actualMenu->process();
		return;
	}

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
