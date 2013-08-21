/*
 * Interface.cpp
 *
 *  Created on: 19-08-2013
 *      Author: michal
 */

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <cstring>

#include <boost/circular_buffer.hpp>
#include <boost/utility/enable_if.hpp>

#include "Interface.hpp"

#include "usb-commands.h"

using namespace std;
using namespace boost;

namespace bridge {

	const double VOLTAGE_FACTOR = .01981590757978723404;
	const double CURRENT_FACTOR = 34.0;

	const int VOLTAGE_ARRAY_SIZE = 5;
	const int CURRENT_ARRAY_SIZE = 5;

	const string KILLSWITCH_STRING = "killswitch";

	class DataHolder {
	private:
		uint8_t* data = nullptr;
		unsigned int length = 0;

		void initData(USBCommands::Request request, unsigned int dataSize = 0) {
			length = dataSize + 1;
			data = new uint8_t[length];
			data[0] = request;
		}
	public:
		DataHolder(USBCommands::Request request) {
			initData(request);
		}

		~DataHolder() {
			delete[] data;
		}

		template<typename TYPE,
			typename = typename std::enable_if<not std::is_same<TYPE, std::vector<uint8_t>>::value>::type>
		DataHolder(USBCommands::Request request, TYPE& structure) {
			initData(request, sizeof(TYPE));

			std::memcpy(data + 1, &structure, sizeof(TYPE));
		}

		DataHolder(USBCommands::Request request, const std::vector<uint8_t>& arr) {
			initData(request, arr.size());

			unsigned int idx = 1;
			for (auto& val : arr) {
				data[idx] = val;
				idx++;
			}
		}

		template<typename ... ARGS> static std::shared_ptr<DataHolder> create(ARGS ... args) {
			return std::shared_ptr<DataHolder>(new DataHolder(args...));
		}

		uint8_t* getPlainData() {
			return data;
		}

		template<typename TYPE> TYPE* getPayload() {
			return reinterpret_cast<TYPE*>(data + 1);
		}

		USBCommands::Request getRequest() {
			return static_cast<USBCommands::Request>(data[0]);
		}

		unsigned int getSize() {
			return length;
		}

		void appendTo(std::vector<uint8_t>& vec) {
			vec.insert(vec.end(), data, data + length);
		}
	};

	struct Implementation: noncopyable {
		unique_ptr<circular_buffer<unsigned int>> rawVoltage;
		unique_ptr<circular_buffer<unsigned int>> rawCurrent;

		string lcdText;

		map<Button, bool> buttons;

		unordered_map<string, std::shared_ptr<DataHolder>> requests;

		bool killSwitchActive;
	};

	Interface::Interface() :
		impl(new Implementation), expander(*(new ExpanderClass(impl))), motor(*(new MotorClass(impl))), arm(
			*(new ArmClass(impl))) {
		impl->rawVoltage.reset(new circular_buffer<unsigned int>(VOLTAGE_ARRAY_SIZE));
		impl->rawCurrent.reset(new circular_buffer<unsigned int>(CURRENT_ARRAY_SIZE));

	}

	Interface::~Interface() {
		delete &arm;
		delete &motor;
		delete &expander;

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

		vector<uint8_t> data = { uint8_t(newString.length()) };

		for (auto character : newString) {
			data.push_back(character);
		}

		impl->requests["lcdtext"] = DataHolder::create(USBCommands::BRIDGE_LCD_SET, data);
	}

	void Interface::setKillSwitch(bool active) {
		USBCommands::bridge::KillSwitch state = USBCommands::bridge::INACTIVE;
		if (active) {
			state = USBCommands::bridge::ACTIVE;
		}
		impl->requests[KILLSWITCH_STRING] = DataHolder::create(USBCommands::BRIDGE_SET_KILLSWITCH, state);
	}

	bool Interface::isKillSwitchActive() {
		USBCommands::bridge::State test;
		test.rawCurrent = 0xffff;
		test.rawVoltage = 0xeeee;

		impl->requests["bridge"] = DataHolder::create(USBCommands::BRIDGE_GET_STATE, test);

		impl->requests["bridge"]->getPayload<USBCommands::bridge::State>()->rawVoltage = 0xaaaa;

		return impl->killSwitchActive;
	}

	bool Interface::isButtonPressed(Button button) {
		return impl->buttons[button];
	}

	void Interface::sendChanges() {
		vector<uint8_t> concatenated;

		// ensure that disabling kill switch is the first command
		auto killSwitchRequest = impl->requests.find(KILLSWITCH_STRING);
		if (killSwitchRequest != impl->requests.end()
			and killSwitchRequest->second->getPlainData()[1] == USBCommands::bridge::INACTIVE) {
			killSwitchRequest->second->appendTo(concatenated);
		}

		for (auto& request : impl->requests) {
			if (request.first == KILLSWITCH_STRING) {
				continue;
			}

			cout << request.first << ": " << request.second->getSize() << endl;
			request.second->appendTo(concatenated);
		}

		if (killSwitchRequest != impl->requests.end()
			and killSwitchRequest->second->getPlainData()[1] == USBCommands::bridge::ACTIVE) {
			killSwitchRequest->second->appendTo(concatenated);
		}

		cout << concatenated.size() << " " << concatenated.capacity() << endl;
		for (unsigned int i = 0; i < concatenated.size(); i++) {
			cout << i << "i: " << (int) concatenated[i] << endl;
		}
	}

	void Interface::MotorClass::SingleMotor::setSpeed(unsigned int speed) {
		string key = "motor_" + to_string(int(getMotor()));

		/*if (impl->requests.find(key) == impl->requests.end()) {

		 }*/

		//impl->requests[key]
	}

	void Interface::MotorClass::SingleMotor::setDirection(Direction direction) {
	}

	void Interface::MotorClass::brake() {
		motors[Motor::LEFT]->setDirection(Direction::STOP);
		motors[Motor::RIGHT]->setDirection(Direction::STOP);
	}

	void Interface::ArmClass::SingleJoint::setSpeed(unsigned int speed) {
	}

	void Interface::ArmClass::SingleJoint::setDirection(Direction direction) {
	}

	void Interface::ArmClass::SingleJoint::setPosition(unsigned int position) {
	}

	void Interface::ArmClass::brake() {
	}

	void Interface::ArmClass::calibrate() {
	}

	void Interface::ExpanderClass::Device::setEnabled(bool enabled) {
	}

} /* namespace bridge */
