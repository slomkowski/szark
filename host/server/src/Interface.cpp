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
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "Interface.hpp"
#include "DataHolder.hpp"

#include "usb-commands.hpp"

using namespace std;
using namespace boost;

namespace bridge {

	const int VOLTAGE_ARRAY_SIZE = 5;
	const int CURRENT_ARRAY_SIZE = 5;

	const uint8_t MOTOR_DRIVER_MAX_SPEED = 11;
	const unsigned int ARM_DRIVER_MAX_SPEED = 255;

	map<bridge::Joint, unsigned int> ARM_DRIVER_MAX_POSITION = {{Joint::ELBOW, 105}, {Joint::SHOULDER, 79},
			{Joint::GRIPPER, 255}};

	log4cpp::Category &Interface::logger = log4cpp::Category::getInstance("Interface");

	enum Priority {
		PRIORITY_KILL_SWITCH,
		PRIORITY_ARM_SET,
		PRIORITY_MOTOR_SET,
		PRIORITY_ARM_GENERAL_SET,
		PRIORITY_EXPANDER_SET,
		PRIORITY_LCD
	};

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
			case Joint::SHOULDER:
			default:
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
		return round(10.0 * USBCommands::bridge::VOLTAGE_FACTOR * accumulate(rawVoltage->begin(), rawVoltage->end(), 0) / rawVoltage->size())
				/ 10.0;
	}

	double Interface::getCurrent() {
		return round(10.0 * USBCommands::bridge::CURRENT_FACTOR * accumulate(rawCurrent->begin(), rawCurrent->end(), 0) / rawCurrent->size())
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

		vector<uint8_t> data = {uint8_t(newString.length())};

		for (auto character : newString) {
			data.push_back(character);
		}

		logger.info(string("Setting LCD text to: \"") + newString + "\"");

		requests["lcdtext"] = DataHolder::create(USBCommands::BRIDGE_LCD_SET, PRIORITY_LCD, false, data);
	}

	void Interface::setKillSwitch(bool active) {
		if (active) {
			logger.info("Setting kill switch to active.");
			killSwitchActive = true;
			killSwitchCausedByHardware = false;
			requests[KILLSWITCH_STRING] = DataHolder::create(USBCommands::BRIDGE_SET_KILLSWITCH, false, PRIORITY_KILL_SWITCH,
					USBCommands::bridge::ACTIVE);
		} else {
			logger.info("Setting kill switch to inactive.");
			requests[KILLSWITCH_STRING] = DataHolder::create(USBCommands::BRIDGE_SET_KILLSWITCH, false, PRIORITY_KILL_SWITCH,
					USBCommands::bridge::INACTIVE);
		}
	}

	bool Interface::isButtonPressed(Button button) {
		return buttons[button];
	}

	void Interface::MotorClass::SingleMotor::onKillSwitchActivated() {
		power = 0;
		programmedSpeed = 0;
		direction = Direction::STOP;
	}

	void Interface::MotorClass::SingleMotor::initStructure() {
		if (requests.find(getKey()) == requests.end()) {
			programmedSpeed = 0;
			direction = Direction::STOP;
			power = 0;
			createMotorState();
		}
	}

	void Interface::MotorClass::SingleMotor::createMotorState() {
		USBCommands::motor::SpecificMotorState mState;
		mState.speed = programmedSpeed;

		switch (motor) {
			case Motor::LEFT:
				mState.motor = motor::MOTOR1;
				break;
			case Motor::RIGHT:
			default:
				mState.motor = motor::MOTOR2;
				break;
		};

		switch (direction) {
			case Direction::STOP:
				mState.direction = motor::STOP;
				break;
			case Direction::FORWARD:
				mState.direction = motor::FORWARD;
				break;
			case Direction::BACKWARD:
				mState.direction = motor::BACKWARD;
				break;
		};

		requests[getKey()] = DataHolder::create(USBCommands::MOTOR_DRIVER_SET, PRIORITY_MOTOR_SET, true, mState);
	}

	void Interface::MotorClass::SingleMotor::setSpeed(unsigned int speed) {
		if (speed <= MOTOR_DRIVER_MAX_SPEED) {
			programmedSpeed = speed;
		} else {
			logger.warn((format("Trying to set greater speed for %s than max: %d.") % devToString(motor) % speed).str());
			programmedSpeed = MOTOR_DRIVER_MAX_SPEED;
		}

		createMotorState();

		logger.info((format("Setting speed of %s to %d.") % devToString(motor) % (int) programmedSpeed).str());
	}

	void Interface::MotorClass::SingleMotor::setDirection(Direction dir) {
		this->direction = dir;

		createMotorState();

		logger.info((format("Setting direction of %s to %s.") % devToString(motor) % directionToString(dir)).str());
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

	void Interface::ArmClass::SingleJoint::initStructure() {
		if (requests.find(getKey()) == requests.end()) {
			direction = Direction::STOP;
			speed = 0;
			settingPosition = false;
			programmedPosition = 0;

			createJointState();
		}
	}

	void Interface::ArmClass::SingleJoint::createJointState() {
		USBCommands::arm::JointState jState;

		switch (joint) {
			case Joint::ELBOW:
				jState.motor = arm::ELBOW;
				break;
			case Joint::GRIPPER:
				jState.motor = arm::GRIPPER;
				break;
			case Joint::SHOULDER:
			default:
				jState.motor = arm::SHOULDER;
				break;
		};

		switch (direction) {
			case Direction::STOP:
				jState.direction = arm::STOP;
				break;
			case Direction::FORWARD:
				jState.direction = arm::FORWARD;
				break;
			case Direction::BACKWARD:
				jState.direction = arm::BACKWARD;
				break;
		};

		jState.speed = speed;
		jState.position = programmedPosition;
		jState.setPosition = settingPosition;

		requests[getKey()] = DataHolder::create(USBCommands::ARM_DRIVER_SET, PRIORITY_ARM_SET, true, jState);
	}

	void Interface::ArmClass::SingleJoint::setSpeed(unsigned int speed) {
		unsigned int effectiveSpeed = 0;
		if (speed <= ARM_DRIVER_MAX_SPEED) {
			effectiveSpeed = speed;
		} else {
			logger.warn(
					(format("Trying to set greater speed for %s than max: %d.") % devToString(joint) % ARM_DRIVER_MAX_SPEED).str());
			effectiveSpeed = ARM_DRIVER_MAX_SPEED;
		}

		this->speed = speed;

		createJointState();

		logger.info((format("Setting speed of %s to %d.") % devToString(joint) % (int) effectiveSpeed).str());
	}

	void Interface::ArmClass::SingleJoint::setDirection(Direction direction) {
		this->direction = direction;
		this->settingPosition = false;
		//TODO ustawianie armDriverMode
		//mode = ArmDriverMode::DIRECTIONAL;

		createJointState();

		logger.info((format("Setting direction of %s to %s.") % devToString(joint) % directionToString(direction)).str());
	}

	void Interface::ArmClass::SingleJoint::setPosition(unsigned int position) {
		unsigned int effectivePos;
		if (position <= ARM_DRIVER_MAX_POSITION[joint]) {
			effectivePos = position;
		} else {
			logger.warn(
					(format("Trying to set greater position for %s than max: %d.") % devToString(joint)
							% ARM_DRIVER_MAX_POSITION[joint]).str());
			effectivePos = ARM_DRIVER_MAX_POSITION[joint];
		}

		this->programmedPosition = effectivePos;
		this->settingPosition = true;
		//TODO ustawianie armDriverMode
		//mode = ArmDriverMode::POSITIONAL;

		createJointState();

		logger.info((format("Setting position of %s to %d.") % devToString(joint) % (int) effectivePos).str());
	}

	void Interface::ArmClass::brake() {
		requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, PRIORITY_ARM_GENERAL_SET, true,
				USBCommands::arm::BRAKE);

		logger.notice("Braking all joints.");
	}

	void Interface::ArmClass::calibrate() {
		//TODO z tym coś zrobić, bo wysyła komendę za każdym razem
		if (calibrationStatus == ArmCalibrationStatus::NONE ||
				calibrationStatus == ArmCalibrationStatus::DONE) {
			requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, PRIORITY_ARM_GENERAL_SET, true,
					USBCommands::arm::CALIBRATE);

			calibrationStatus = ArmCalibrationStatus::IN_PROGRESS;
			mode = ArmDriverMode::CALIBRATING;
			logger.notice("Begining calibrating arm driver.");
		}
	}

	void Interface::ExpanderClass::Device::setEnabled(bool enabled) {
		if (enabled) {
			expanderByte |= (1 << int(device));
			logger.info(string("Enabling expander device ") + devToString(device) + ".");
		} else {
			expanderByte &= ~(1 << int(device));
			logger.info(string("Disabling expander device ") + devToString(device) + ".");
		}

		requests["expander"] = DataHolder::create(USBCommands::EXPANDER_SET, PRIORITY_EXPANDER_SET, true, expanderByte);
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

	unsigned int Interface::ExpanderClass::updateFields(USBCommands::Request request, uint8_t *data) {
		if (request != USBCommands::EXPANDER_GET) {
			return 0;
		}

		expanderByte = data[0];

		logger.info("Updating state expander byte.");

		return 1;
	}

	unsigned int Interface::ArmClass::updateFields(USBCommands::Request request, uint8_t *data) {
		if (request != USBCommands::ARM_DRIVER_GET_GENERAL_STATE) {
			return 0;
		}

		auto state = reinterpret_cast<USBCommands::arm::GeneralState *>(data);

		if (state->isCalibrated) {
			if (calibrationStatus == ArmCalibrationStatus::IN_PROGRESS) {
				requests.erase("arm_addon");
				logger.notice("Calibration finished.");
			}
			calibrationStatus = ArmCalibrationStatus::DONE;
		}

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

	unsigned int Interface::MotorClass::SingleMotor::updateFields(USBCommands::Request request, uint8_t *data) {
		if (request != USBCommands::MOTOR_DRIVER_GET) {
			return 0;
		}

		auto state = reinterpret_cast<USBCommands::motor::SpecificMotorState *>(data);

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

		// direction field has no meaning value
//	switch (state->direction) {
//	case motor::FORWARD:
//		direction = Direction::FORWARD;
//		break;
//	case motor::BACKWARD:
//		direction = Direction::BACKWARD;
//		break;
//	default:
//		direction = Direction::STOP;
//		break;
//	};

		power = state->speed;

		logger.info((format("Updating state motor %s power: %d.") % devToString(motorNo) % (int) power).str());

		return sizeof(USBCommands::motor::SpecificMotorState);
	}

	unsigned int Interface::ArmClass::SingleJoint::updateFields(USBCommands::Request request, uint8_t *data) {
		if (request != USBCommands::ARM_DRIVER_GET) {
			return 0;
		}

		auto state = reinterpret_cast<USBCommands::arm::JointState *>(data);

		Joint jointNo = Joint::SHOULDER;

		switch (state->motor) {
			case arm::ELBOW:
				jointNo = Joint::ELBOW;
				break;
			case arm::GRIPPER:
				jointNo = Joint::GRIPPER;
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

	unsigned int Interface::updateFields(USBCommands::Request request, uint8_t *data) {
		if (request != USBCommands::BRIDGE_GET_STATE) {
			return 0;
		}

		auto state = reinterpret_cast<USBCommands::bridge::State *>(data);

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

		unsigned int curr = state->rawCurrent;
		unsigned int volt = state->rawVoltage;

		rawCurrent->push_back(curr);
		rawVoltage->push_back(volt);

		logger.debug((format("Raw voltage: %u, raw current: %u.") % volt % curr).str());

		logger.info(
				(format("Updating state: kill switch: %d (by hardware: %d), battery: %.1fV, %.1fA") % killSwitchActive
						% killSwitchCausedByHardware % getVoltage() % getCurrent()).str());

		return sizeof(USBCommands::bridge::State);
	}

	void Interface::ArmClass::onKillSwitchActivated() {
		mode = ArmDriverMode::DIRECTIONAL;

		if (calibrationStatus == ArmCalibrationStatus::IN_PROGRESS) {
			calibrationStatus = ArmCalibrationStatus::NONE;
		}

		requests.erase("arm_addon");
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
			case Direction::FORWARD:
				return "forward";
			case Direction::BACKWARD:
				return "backward";
			case Direction::STOP:
			default:
				return "stop";
		}
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

	std::string armDriverModeToString(const ArmDriverMode mode) {
		switch (mode) {
			case ArmDriverMode::CALIBRATING:
				return "calibrating";
			case ArmDriverMode::POSITIONAL:
				return "positional";
			case ArmDriverMode::DIRECTIONAL:
			default:
				return "directional";
		}
	}

	std::string armCalibrationStatusToString(const ArmCalibrationStatus status) {
		switch (status) {
			case ArmCalibrationStatus::DONE:
				return "done";
			case ArmCalibrationStatus::IN_PROGRESS:
				return "prog";
			case ArmCalibrationStatus::NONE:
			default:
				return "none";
		}
	}

} /* namespace bridge */
