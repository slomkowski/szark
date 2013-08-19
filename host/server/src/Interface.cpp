/*
 * Interface.cpp
 *
 *  Created on: 19-08-2013
 *      Author: michal
 */

#include <cstdint>
#include <unordered_map>
#include <boost/circular_buffer.hpp>

#include "Interface.hpp"

#include "usb-commands.h"

using namespace std;
using namespace boost;

namespace bridge {

	const double VOLTAGE_FACTOR = .01981590757978723404;
	const double CURRENT_FACTOR = 34.0;

	const int VOLTAGE_ARRAY_SIZE = 5;
	const int CURRENT_ARRAY_SIZE = 5;

	struct Implementation: noncopyable {
		unique_ptr<circular_buffer<unsigned int>> rawVoltage;
		unique_ptr<circular_buffer<unsigned int>> rawCurrent;

		string lcdText;

		map<Button, bool> buttons;

		bool killSwitchActive;
	};

	Interface::Interface() {
		impl = new Implementation;

		impl->rawVoltage.reset(new circular_buffer<unsigned int>(VOLTAGE_ARRAY_SIZE));
		impl->rawCurrent.reset(new circular_buffer<unsigned int>(CURRENT_ARRAY_SIZE));
	}

	Interface::~Interface() {
		delete impl;
	}

	double Interface::getVoltage() {
		return VOLTAGE_FACTOR * accumulate(impl->rawVoltage->begin(), impl->rawVoltage->end(), 0)
			/ impl->rawVoltage->size();
	}

	double Interface::getCurrent() {
		return CURRENT_FACTOR * accumulate(impl->rawCurrent->begin(), impl->rawCurrent->end(), 0)
			/ impl->rawCurrent->size();
	}

	std::string Interface::getLCDText() {
		return impl->lcdText;
	}

	void Interface::setLCDText(std::string text) {
		// 32 characters + new line
		if (text.length() > 33) {
			impl->lcdText = text.substr(0, 33);
		} else {
			impl->lcdText = text;
		}

		// TODO push to output buffer
	}

	void Interface::setKillSwitch(bool active) {
		// TODO push to output buffer
	}

	bool Interface::isKillSwitchActive() {
		return impl->killSwitchActive;
	}

	bool Interface::isButtonPressed(Button button) {
		return impl->buttons[button];
	}

} /* namespace bridge */
