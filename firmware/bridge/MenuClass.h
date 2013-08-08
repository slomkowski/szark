/*
 * MenuClass.h
 *
 *  Created on: 08-08-2013
 *      Author: michal
 */

#ifndef MENUCLASS_H_
#define MENUCLASS_H_

#include <stdlib.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "buttons.h"

namespace menu {

	class MenuClass;

	struct MenuItem {
		const char *name PROGMEM;
		MenuClass *subMenu;
	};

	class MenuClass {
	public:
		MenuClass(const char* title, uint8_t itemsLength, MenuItem menuItems[] = NULL, MenuClass *parent = NULL);
		void process();

		void setParent(MenuClass *parent) {
			this->parent = parent;
		}

		void setSubMenuFunction(void (*subMenuFunction)(uint8_t currentPosition, buttons::Buttons *buttonsState)) {
			this->subMenuFunction = subMenuFunction;
		}

		void setHeaderFunction(void (*headerFunction)()) {
			this->headerFunction = headerFunction;
		}

	private:
		const char *title;
		const MenuItem *menuItems;
		MenuClass *parent;
		uint8_t currentPosition;
		uint8_t itemsLength;
		bool inSubMenuFunction;
		static MenuClass *actualMenu;

		void (*headerFunction)();
		void (*subMenuFunction)(uint8_t currentPosition, buttons::Buttons *buttonsState);
	};

} /* namespace faza */
#endif /* MENUCLASS_H_ */
