/*
 * Interface.cpp
 *
 *  Created on: 19-08-2013
 *      Author: michal
 */

#include <cstdint>
#include <vector>
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

	typedef vector<uint8_t> DataHolder;

	/*class DataHolder: public DataHolder {
	 public:
	 template<typename T> void insert(T& structure) {
	 uint8_t* data = reinterpret_cast<uint8_t*>(&structure);

	 for (int i = 0; i < sizeof(T); i++) {
	 this->push_back(data[i]);
	 }
	 }
	 };*/

	struct Implementation: noncopyable {
		unique_ptr<circular_buffer<unsigned int>> rawVoltage;
		unique_ptr<circular_buffer<unsigned int>> rawCurrent;

		string lcdText;

		map<Button, bool> buttons;

		map<string, DataHolder> requests;

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
		string newString;
		// 32 characters + new line
		if (text.length() > 33) {
			newString = text.substr(0, 33);
		} else {
			newString = text;
		}

		if (newString == impl->lcdText) {
			return;
		}

		impl->lcdText = newString;

		DataHolder data = { USBCommands::BRIDGE_LCD_SET, uint8_t(newString.length()) };

		for (auto character : newString) {
			data.push_back(character);
		}

		impl->requests["lcdtext"] = data;
	}

	void Interface::setKillSwitch(bool active) {
		USBCommands::bridge::KillSwitch state = USBCommands::bridge::INACTIVE;
		if (active) {
			state = USBCommands::bridge::ACTIVE;
		}
		impl->requests["killswitch"] = DataHolder( { USBCommands::BRIDGE_SET_KILLSWITCH, state });
	}

	bool Interface::isKillSwitchActive() {
		return impl->killSwitchActive;
	}

	bool Interface::isButtonPressed(Button button) {
		return impl->buttons[button];
	}

	void Interface::sendChanges() {
		vector<uint8_t> concatenated;

		for (auto& request : impl->requests) {
			cout << request.first << " " << request.second.size() << endl;
			concatenated.insert(concatenated.end(), request.second.begin(), request.second.end());
		}

		cout << concatenated.size() << " " << concatenated.capacity() << endl;
	}

} /* namespace bridge */
