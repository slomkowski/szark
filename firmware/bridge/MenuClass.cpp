/*
 * MenuClass.cpp
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#include "global.hpp"

#include <avr/pgmspace.h>
#include <stdlib.h>

#include "lcd.hpp"
#include "buttons.hpp"
#include "killswitch.hpp"
#include "MenuClass.hpp"

using namespace menu;

MenuClass *MenuClass::actualMenu;

MenuClass::MenuClass(const char* title, uint8_t itemsLength, MenuItem menuItems[], MenuClass *parent) {
	this->title = title;
	this->menuItems = menuItems;
	this->itemsLength = itemsLength;
	this->parent = parent;

	this->currentPosition = 0;
	this->headerFunction = nullptr;
	this->subMenuFunction = nullptr;
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
		if (subMenuFunction == nullptr) {
			lcd::putsp(PSTR("No sub-menu function nor menu items!"));
		} else {
			if (not inSubMenuFunction) {
				killswitch::setActive(false);
			}

			if (killswitch::isActive() == true) {
				lcd::putsp(PSTR("  KILL  SWITCH\n    PRESSED!"));
			} else {
				(*subMenuFunction)(not inSubMenuFunction, this->currentPosition, buttons);
			}

			inSubMenuFunction = true;
		}
	} else {
		if (this->title != nullptr) {
			lcd::puts(title);
		} else if (this->headerFunction != nullptr) {
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
				if ((parent == nullptr and currentPosition == itemsLength - 1) or currentPosition == itemsLength) {
					currentPosition = 0;
				} else {
					currentPosition++;
				}
			} else if (buttons->down) {
				if (currentPosition == 0) {
					if (parent == nullptr) {
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
		if (itemsLength > 0 and currentPosition == itemsLength) {
			actualMenu = parent;
		} else {
			if (subMenuFunction != nullptr) {
				if (not inSubMenuFunction) {
					inSubMenuFunction = true;
					killswitch::setActive(false);
					lcd::clrscr();
					(*subMenuFunction)(true, this->currentPosition, buttons);
				} else {
					inSubMenuFunction = false;
					killswitch::setActive(true);
					if (itemsLength == 0) {
						actualMenu = parent;
					}
				}
			} else if (menuItems[currentPosition].subMenu != nullptr) {
				actualMenu = menuItems[currentPosition].subMenu;
				actualMenu->currentPosition = 0;
			} else {
				lcd::clrscr();
				lcd::putsp(PSTR("No function nor submenu!"));
			}
		}
	}
}
