/*
 * Interface.cpp
 *
 *  Created on: 19-08-2013
 *      Author: michal
 */

#include <cstdint>

#include "Interface.hpp"

#include "usb-commands.h"

namespace bridge {

	struct Implementation {
	};

	Interface::Interface() {
		impl = new Implementation;
	}

	Interface::~Interface() {
		delete impl;
	}

	double Interface::getVoltage() {
	}

	double Interface::getCurrent() {
	}

	std::string Interface::getLCDText() {
	}

	void Interface::setLCDText(std::string text) {
	}

	void Interface::setKillSwitch(bool active) {
	}

	bool Interface::isKillSwitchActive() {
	}

	bool Interface::isButtonPressed(Button button) {
	}

} /* namespace bridge */
