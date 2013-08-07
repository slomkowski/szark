/*
 * menu.h
 *
 *  Created on: 07-08-2013
 *      Author: michal
 */

#ifndef MENU_H_
#define MENU_H_

#include <stdlib.h>
#include <stdint.h>
#include <avr/pgmspace.h>

namespace menu {

	struct Menu;

	struct MenuItem {
		const char *name PROGMEM;
		Menu *subMenu;
	};

	class Menu {
	public:
		Menu(const char* title, uint8_t itemsLength, MenuItem menuItems[] = NULL, Menu *parent = NULL);
		void process();

		void setParent(Menu *parent) {
			this->parent = parent;
		}

		void setSubMenuFunction(void (*subMenuFunction)(uint8_t currentPosition)) {
			this->subMenuFunction = subMenuFunction;
		}

		void setHeaderFunction(void (*headerFunction)()) {
			this->headerFunction = headerFunction;
		}

	private:
		const char *title;
		const MenuItem *menuItems;
		Menu *parent;
		uint8_t currentPosition = 0;
		uint8_t itemsLength;

		void (*headerFunction)() = NULL;
		void (*subMenuFunction)(uint8_t currentPosition) = NULL;
	};

	void init();

	void poll();
}

#endif /* MENU_H_ */
