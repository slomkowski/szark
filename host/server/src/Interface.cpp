/*
 * Interface.cpp
 *
 *  Created on: 19-08-2013
 *      Author: michal
 */

#include <cstdint>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

#include "Interface.hpp"
#include "DataHolder.hpp"

#include "usb-commands.hpp"

using namespace std;
using namespace boost;

namespace bridge {

const double VOLTAGE_FACTOR = .01981590757978723404;
const double CURRENT_FACTOR = 34.0;

const int VOLTAGE_ARRAY_SIZE = 5;
const int CURRENT_ARRAY_SIZE = 5;

const uint8_t MOTOR_DRIVER_MAX_SPEED = 12;
const unsigned int ARM_DRIVER_MAX_SPEED = 255;

map<bridge::Joint, unsigned int> ARM_DRIVER_MAX_POSITION = { { Joint::ELBOW, 105 }, { Joint::SHOULDER, 79 }, {
		Joint::WRIST, 95 }, { Joint::GRIPPER, 255 } };

Interface::Interface()
		: expander(*(new ExpanderClass(requests))), motor(*(new MotorClass(requests))), arm(*(new ArmClass(requests))) {

	rawVoltage.reset(new circular_buffer<unsigned int>(VOLTAGE_ARRAY_SIZE));
	rawCurrent.reset(new circular_buffer<unsigned int>(CURRENT_ARRAY_SIZE));

	rawVoltage->push_back(0);
	rawCurrent->push_back(0);

	extDevListeners.push_back(this);
	extDevListeners.push_back(&arm);
	extDevListeners.push_back(&expander);
	for (auto d : arm.joints) {
		extDevListeners.push_back(d.second.get());
	}
	for (auto d : motor.motors) {
		extDevListeners.push_back(d.second.get());
	}

	killSwitchActive = true;
	setKillSwitch(true);
}

Interface::~Interface() {
	delete &arm;
	delete &motor;
	delete &expander;
}

double Interface::getVoltage() {
	return round(10.0 * VOLTAGE_FACTOR * accumulate(rawVoltage->begin(), rawVoltage->end(), 0) / rawVoltage->size())
			/ 10.0;
}

double Interface::getCurrent() {
	return round(10.0 * CURRENT_FACTOR * accumulate(rawCurrent->begin(), rawCurrent->end(), 0) / rawCurrent->size())
			/ 10.0;
}

void Interface::setLCDText(std::string text) {
	string newString;
	// 32 characters + new line
	if (text.length() > 33) {
		newString = text.substr(0, 33);
	} else {
		newString = text;
	}

	if (newString == lcdText) {
		return;
	}

	lcdText = newString;

	vector<uint8_t> data = { uint8_t(newString.length()) };

	for (auto character : newString) {
		data.push_back(character);
	}

	requests["lcdtext"] = DataHolder::create(USBCommands::BRIDGE_LCD_SET, data);
}

void Interface::setKillSwitch(bool active) {
	USBCommands::bridge::KillSwitch state = USBCommands::bridge::INACTIVE;
	if (active) {
		state = USBCommands::bridge::ACTIVE;
	}
	requests[KILLSWITCH_STRING] = DataHolder::create(USBCommands::BRIDGE_SET_KILLSWITCH, state);
}

bool Interface::isButtonPressed(Button button) {
	return buttons[button];
}

void Interface::MotorClass::SingleMotor::onKillSwitchActivated() {
	power = 0;
	programmedSpeed = 0;
	direction = Direction::STOP;
}

string Interface::MotorClass::SingleMotor::initStructure() {
	string key = "motor_" + to_string(int(motor));
	if (requests.find(key) == requests.end()) {
		USBCommands::motor::SpecificMotorState mState;
		switch (motor) {
		case Motor::LEFT:
			mState.motor = motor::MOTOR1;
			break;
		case Motor::RIGHT:
		default:
			mState.motor = motor::MOTOR2;
			break;
		};
		mState.direction = motor::STOP;
		mState.speed = 0;
		requests[key] = DataHolder::create(USBCommands::MOTOR_DRIVER_SET, mState);
	}
	return key;
}

void Interface::MotorClass::SingleMotor::setSpeed(unsigned int speed) {

	string key = initStructure();

	programmedSpeed = speed <= MOTOR_DRIVER_MAX_SPEED ? speed : MOTOR_DRIVER_MAX_SPEED;

	requests[key]->getPayload<USBCommands::motor::SpecificMotorState>()->speed = programmedSpeed;
}

void Interface::MotorClass::SingleMotor::setDirection(Direction direction) {
	string key = initStructure();

	motor::Direction dir = motor::STOP;

	switch (direction) {
	case Direction::STOP:
		dir = motor::STOP;
		break;
	case Direction::FORWARD:
		dir = motor::FORWARD;
		break;
	case Direction::BACKWARD:
		dir = motor::BACKWARD;
		break;
	};

	requests[key]->getPayload<USBCommands::motor::SpecificMotorState>()->direction = dir;
}

void Interface::MotorClass::brake() {
	motors[Motor::LEFT]->setDirection(Direction::STOP);
	motors[Motor::RIGHT]->setDirection(Direction::STOP);
}

void Interface::ArmClass::SingleJoint::onKillSwitchActivated() {
	direction = Direction::STOP;
	speed = 0;
}

string Interface::ArmClass::SingleJoint::initStructure() {
	string key = "arm_" + to_string(int(joint));
	if (requests.find(key) == requests.end()) {
		USBCommands::arm::JointState jState;

		switch (joint) {
		case Joint::ELBOW:
			jState.motor = arm::ELBOW;
			break;
		case Joint::WRIST:
			jState.motor = arm::WRIST;
			break;
		case Joint::GRIPPER:
			jState.motor = arm::GRIPPER;
			break;
		case Joint::SHOULDER:
		default:
			jState.motor = arm::SHOULDER;
			break;
		};

		jState.direction = arm::STOP;
		jState.speed = 0;
		jState.position = 0;
		jState.setPosition = false;

		requests[key] = DataHolder::create(USBCommands::ARM_DRIVER_SET, jState);
	}

	return key;
}

void Interface::ArmClass::SingleJoint::setSpeed(unsigned int speed) {
	string key = initStructure();

	requests[key]->getPayload<USBCommands::arm::JointState>()->speed =
			speed <= ARM_DRIVER_MAX_SPEED ? speed : ARM_DRIVER_MAX_SPEED;
}

void Interface::ArmClass::SingleJoint::setDirection(Direction direction) {
	string key = initStructure();

	arm::Direction dir = arm::STOP;

	switch (direction) {
	case Direction::STOP:
		dir = arm::STOP;
		break;
	case Direction::FORWARD:
		dir = arm::FORWARD;
		break;
	case Direction::BACKWARD:
		dir = arm::BACKWARD;
		break;
	};

	auto state = requests[key]->getPayload<USBCommands::arm::JointState>();
	state->direction = dir;
	state->setPosition = false;
}

void Interface::ArmClass::SingleJoint::setPosition(unsigned int position) {
	string key = initStructure();

	auto state = requests[key]->getPayload<USBCommands::arm::JointState>();

	state->position = position <= ARM_DRIVER_MAX_POSITION[joint] ? position : ARM_DRIVER_MAX_POSITION[joint];
	state->setPosition = true;
}

void Interface::ArmClass::brake() {
	requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, USBCommands::arm::BRAKE);
}

void Interface::ArmClass::calibrate() {
	requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, USBCommands::arm::CALIBRATE);
}

void Interface::ExpanderClass::Device::setEnabled(bool enabled) {
	if (enabled) {
		expanderByte |= (1 << int(device));
	} else {
		expanderByte &= ~(1 << int(device));
	}

	requests["expander"] = DataHolder::create(USBCommands::EXPANDER_SET, expanderByte);
}

bool Interface::ExpanderClass::Device::isEnabled() {
	return ((1 << int(device)) & expanderByte);
}

void Interface::updateDataStructures(std::vector<USBCommands::Request> getterRequests,
		std::vector<uint8_t> deviceResponse) {

	unsigned int actualPosition = 0;

	for (auto gReq : getterRequests) {
		unsigned int bytesTaken = 0;

		for (auto listener : extDevListeners) {
			bytesTaken = listener->updateFields(gReq, &deviceResponse[actualPosition]);

			if (bytesTaken > 0) {
				actualPosition += bytesTaken;
				break;
			}
		}

		if (bytesTaken == 0) {
			throw runtime_error(string("Request ") + to_string(int(gReq)) + " hasn't been handled by any listener");
		}
	}

	if (deviceResponse[actualPosition] != USBCommands::MESSAGE_END) {
		auto foundPos = std::distance(deviceResponse.begin(),
				std::find(deviceResponse.begin() + actualPosition, deviceResponse.end(), USBCommands::MESSAGE_END));

		throw runtime_error(
				string("Message finish not found in the response at position: ") + to_string(actualPosition) + ", but: "
						+ to_string(foundPos));
	}
}

unsigned int Interface::ExpanderClass::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::EXPANDER_GET) {
		return 0;
	}

	expanderByte = data[0];

	return 1;
}

unsigned int Interface::ArmClass::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::ARM_DRIVER_GET_GENERAL_STATE) {
		return 0;
	}

	auto state = reinterpret_cast<USBCommands::arm::GeneralState*>(data);

	calibrated = state->isCalibrated;

	switch (state->mode) {
	case arm::DIR:
		mode = ArmDriverMode::DIRECTIONAL;
		break;
	case arm::POS:
		mode = ArmDriverMode::POSITIONAL;
		break;
	case arm::CAL:
		mode = ArmDriverMode::CALIBRATING;
		break;
	};

	return sizeof(USBCommands::arm::GeneralState);
}

unsigned int Interface::MotorClass::SingleMotor::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::MOTOR_DRIVER_GET) {
		return 0;
	}

	auto state = reinterpret_cast<USBCommands::motor::SpecificMotorState*>(data);

	Motor motorNo;
	switch (state->motor) {
	case motor::MOTOR1:
		motorNo = Motor::LEFT;
		break;
	default:
		motorNo = Motor::RIGHT;
		break;
	}

	if (motorNo != motor) {
		return 0;
	}

	/*
	 switch (state->direction) {
	 case Direction::STOP:
	 case motor::BACKWARD:
	 direction = Direction::FORWARD;
	 break;
	 case motor::BACKWARD:
	 direction = Direction::BACKWARD;
	 break;
	 default:
	 direction = Direction::STOP;
	 break;
	 };*/

	power = state->speed;

	return sizeof(USBCommands::motor::SpecificMotorState);
}

unsigned int Interface::ArmClass::SingleJoint::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::ARM_DRIVER_GET) {
		return 0;
	}

	auto state = reinterpret_cast<USBCommands::arm::JointState*>(data);

	Joint jointNo = Joint::SHOULDER;

	switch (state->motor) {
	case arm::ELBOW:
		jointNo = Joint::ELBOW;
		break;
	case arm::GRIPPER:
		jointNo = Joint::GRIPPER;
		break;
	case arm::WRIST:
		jointNo = Joint::WRIST;
		break;
	case arm::SHOULDER:
		jointNo = Joint::SHOULDER;
		break;
	};

	if (jointNo != joint) {
		return 0;
	}

	switch (state->direction) {
	case arm::FORWARD:
		direction = Direction::FORWARD;
		break;
	case arm::BACKWARD:
		direction = Direction::BACKWARD;
		break;
	default:
		direction = Direction::STOP;
		break;
	};

	speed = state->speed;
	position = state->position;

	return sizeof(USBCommands::arm::JointState);
}

unsigned int Interface::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::BRIDGE_GET_STATE) {
		return 0;
	}

	auto state = reinterpret_cast<USBCommands::bridge::State*>(data);

	if (state->killSwitch == USBCommands::bridge::ACTIVE) {
		killSwitchActive = true;
		updateStructsWhenKillSwitchActivated();
	} else {
		killSwitchActive = false;
	}

	buttons[Button::UP] = state->buttonUp;
	buttons[Button::DOWN] = state->buttonDown;
	buttons[Button::ENTER] = state->buttonEnter;

	rawCurrent->push_back(state->rawCurrent);
	rawVoltage->push_back(state->rawVoltage);

	return sizeof(USBCommands::bridge::State);
}

void Interface::ArmClass::onKillSwitchActivated() {
	mode = ArmDriverMode::DIRECTIONAL;
}

void Interface::ExpanderClass::onKillSwitchActivated() {
}

void Interface::updateStructsWhenKillSwitchActivated() {
	for (auto listener : extDevListeners) {
		listener->onKillSwitchActivated();
	}
}

void Interface::onKillSwitchActivated() {
	lcdText = "";
}

} /* namespace bridge */
