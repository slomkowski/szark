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

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

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

log4cpp::Category& Interface::logger = log4cpp::Category::getInstance("Interface");

std::string devToString(ExpanderDevice dev) {
	switch (dev) {
	case ExpanderDevice::LIGHT_CAMERA:
		return "camera light";
	case ExpanderDevice::LIGHT_LEFT:
		return "left light";
	default:
		return "right light";
	};
}

std::string devToString(Joint dev) {
	switch (dev) {
	case Joint::ELBOW:
		return "elbow";
	case Joint::GRIPPER:
		return "gripper";
	case Joint::WRIST:
		return "wrist";
	case Joint::SHOULDER:
		return "shoulder";
	};
}
std::string devToString(Motor dev) {
	if (dev == Motor::LEFT) {
		return "left";
	} else {
		return "right";
	}
}

Interface::Interface()
		:
				expander(*(new ExpanderClass(requests))),
				motor(*(new MotorClass(requests))),
				arm(*(new ArmClass(requests))) {

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
	killSwitchCausedByHardware = false;
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

	logger.info(string("Setting LCD text to: \"") + newString + "\"");

	requests["lcdtext"] = DataHolder::create(USBCommands::BRIDGE_LCD_SET, data);
}

void Interface::setKillSwitch(bool active) {
	USBCommands::bridge::KillSwitch state = USBCommands::bridge::INACTIVE;
	if (active) {
		state = USBCommands::bridge::ACTIVE;
		logger.info("Setting kill switch to active.");
	} else {
		logger.info("Setting kill switch to inactive.");
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

	if (speed <= MOTOR_DRIVER_MAX_SPEED) {
		programmedSpeed = speed;
	} else {
		logger.warn((format("Trying to set greater speed for %s than max: %d.") % devToString(motor) % speed).str());
		programmedSpeed = MOTOR_DRIVER_MAX_SPEED;
	}

	requests[key]->getPayload<USBCommands::motor::SpecificMotorState>()->speed = programmedSpeed;

	logger.info((format("Setting speed of %s to %d.") % devToString(motor) % (int) programmedSpeed).str());
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

	logger.info((format("Setting direction of %s to %s.") % devToString(motor) % directionToString(direction)).str());
}

void Interface::MotorClass::brake() {
	motors[Motor::LEFT]->setDirection(Direction::STOP);
	motors[Motor::RIGHT]->setDirection(Direction::STOP);

	logger.info("Braking all motors.");
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

	unsigned int effectiveSpeed = 0;
	if (speed <= ARM_DRIVER_MAX_SPEED) {
		effectiveSpeed = speed;
	} else {
		logger.warn(
				(format("Trying to set greater speed for %s than max: %d.") % devToString(joint) % ARM_DRIVER_MAX_SPEED).str());
		effectiveSpeed = ARM_DRIVER_MAX_SPEED;
	}

	requests[key]->getPayload<USBCommands::arm::JointState>()->speed = effectiveSpeed;

	logger.info((format("Setting speed of %s to %d.") % devToString(joint) % (int) effectiveSpeed).str());
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

	logger.info((format("Setting direction of %s to %s.") % devToString(joint) % directionToString(direction)).str());
}

void Interface::ArmClass::SingleJoint::setPosition(unsigned int position) {
	string key = initStructure();

	auto state = requests[key]->getPayload<USBCommands::arm::JointState>();

	unsigned int effectivePos;
	if (position <= ARM_DRIVER_MAX_POSITION[joint]) {
		effectivePos = position;
	} else {
		logger.warn(
				(format("Trying to set greater position for %s than max: %d.") % devToString(joint)
						% ARM_DRIVER_MAX_POSITION[joint]).str());
		effectivePos = ARM_DRIVER_MAX_POSITION[joint];
	}

	state->position = effectivePos;
	state->setPosition = true;

	logger.info((format("Setting position of %s to %d.") % devToString(joint) % (int) effectivePos).str());
}

void Interface::ArmClass::brake() {
	requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, USBCommands::arm::BRAKE);

	logger.info("Braking all joints.");
}

void Interface::ArmClass::calibrate() {
	requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, USBCommands::arm::CALIBRATE);
	logger.info("Calibrating arm.");
}

void Interface::ExpanderClass::Device::setEnabled(bool enabled) {
	if (enabled) {
		expanderByte |= (1 << int(device));
		logger.info(string("Enabling expander device ") + devToString(device));
	} else {
		expanderByte &= ~(1 << int(device));
		logger.info(string("Disabling expander device ") + devToString(device));
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
				string("MESSAGE_END not found in the response at position: ") + to_string(actualPosition) + ", but: "
						+ to_string(foundPos));
	}
}

unsigned int Interface::ExpanderClass::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::EXPANDER_GET) {
		return 0;
	}

	expanderByte = data[0];

	logger.info("Updating state expander byte.");

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

	switch (state->direction) {
	case motor::FORWARD:
		direction = Direction::FORWARD;
		break;
	case motor::BACKWARD:
		direction = Direction::BACKWARD;
		break;
	default:
		direction = Direction::STOP;
		break;
	};

	power = state->speed;

	logger.info(
			(format("Updating state motor %s: direction: %s, power: %d.") % devToString(motorNo)
					% directionToString(direction)
					% (int) power).str());

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

	logger.info(
			(format("Updating state joint %s: direction: %s, position: %d.") % devToString(jointNo)
					% directionToString(direction)
					% (int) position).str());

	return sizeof(USBCommands::arm::JointState);
}

unsigned int Interface::updateFields(USBCommands::Request request, uint8_t* data) {
	if (request != USBCommands::BRIDGE_GET_STATE) {
		return 0;
	}

	auto state = reinterpret_cast<USBCommands::bridge::State*>(data);

	if (state->killSwitch == USBCommands::bridge::ACTIVE) {
		killSwitchActive = true;
		killSwitchCausedByHardware = state->killSwitchCausedByHardware;
		updateStructsWhenKillSwitchActivated();
	} else {
		killSwitchCausedByHardware = false;
		killSwitchActive = false;
	}

	buttons[Button::UP] = state->buttonUp;
	buttons[Button::DOWN] = state->buttonDown;
	buttons[Button::ENTER] = state->buttonEnter;

	rawCurrent->push_back(state->rawCurrent);
	rawVoltage->push_back(state->rawVoltage);

	logger.info(
			(format("Updating state: kill switch: %d (by hardware: %d), battery: %.1fV, %.1fA") % killSwitchActive
					% killSwitchCausedByHardware % getVoltage() % getCurrent()).str());

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

std::string directionToString(const Direction dir) {
	switch (dir) {
	case Direction::STOP:
		return "stop";
	case Direction::FORWARD:
		return "forward";
	case Direction::BACKWARD:
		return "backward";
	}

	return "";
}

Direction stringToDirection(std::string dir) {
	boost::algorithm::to_lower(dir);

	if (dir == "stop") {
		return Direction::STOP;
	} else if (dir == "forward") {
		return Direction::FORWARD;
	} else if (dir == "backward") {
		return Direction::BACKWARD;
	} else {
		throw runtime_error("invalid direction: " + dir);
	}

	return Direction::STOP;
}

} /* namespace bridge */
