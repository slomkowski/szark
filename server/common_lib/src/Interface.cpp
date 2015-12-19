#include "Interface.hpp"
#include "convert.hpp"

#include "usb-commands.hpp"

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace common;

constexpr int VOLTAGE_ARRAY_SIZE = 5;
constexpr int CURRENT_ARRAY_SIZE = 5;

constexpr uint8_t MOTOR_DRIVER_MAX_SPEED = 11;
constexpr unsigned int ARM_DRIVER_MAX_SPEED = 255;

static map<bridge::Joint, unsigned int> ARM_DRIVER_MAX_POSITION = {
        {bridge::Joint::ELBOW,    105},
        {bridge::Joint::SHOULDER, 79},
        {bridge::Joint::GRIPPER,  255}
};

enum Priority {
    PRIORITY_KILL_SWITCH,
    PRIORITY_ARM_SET,
    PRIORITY_MOTOR_SET,
    PRIORITY_ARM_GENERAL_SET,
    PRIORITY_EXPANDER_SET,
    PRIORITY_LCD
};

bridge::Interface::Interface()
        : logger(log4cpp::Category::getInstance("Interface")),
          rawVoltage(VOLTAGE_ARRAY_SIZE),
          rawCurrent(CURRENT_ARRAY_SIZE),
          expander(requests, logger),
          motor(requests, logger),
          arm(requests, logger) {

    rawVoltage.push_back(0);
    rawCurrent.push_back(0);

    extDevListeners.push_back(this);
    extDevListeners.push_back(&arm);
    extDevListeners.push_back(&expander);
    for (auto d : arm.joints) {
        extDevListeners.push_back(d.second);
    }
    for (auto d : motor.motors) {
        extDevListeners.push_back(d.second);
    }

    killSwitchActive = true;
    killSwitchCausedByHardware = false;
    setKillSwitch(true);
}

bridge::Interface::~Interface() {
    delete &arm;
    delete &motor;
    delete &expander;
}

double bridge::Interface::getVoltage() {
    return round(10.0 * USBCommands::bridge::VOLTAGE_FACTOR
                 * accumulate(rawVoltage.begin(), rawVoltage.end(), 0) / rawVoltage.size()) / 10.0;
}

double bridge::Interface::getCurrent() {
    return round(10.0 * USBCommands::bridge::CURRENT_FACTOR
                 * accumulate(rawCurrent.begin(), rawCurrent.end(), 0) / rawCurrent.size()) / 10.0;
}

void bridge::Interface::setLCDText(std::string text) {
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

void bridge::Interface::setKillSwitch(bool active) {
    if (active) {
        logger.info("Setting kill switch to active.");
        killSwitchActive = true;
        killSwitchCausedByHardware = false;
        requests[KILLSWITCH_STRING] = DataHolder::create(USBCommands::BRIDGE_SET_KILLSWITCH, false,
                                                         PRIORITY_KILL_SWITCH,
                                                         USBCommands::bridge::ACTIVE);
    } else {
        logger.info("Setting kill switch to inactive.");
        requests[KILLSWITCH_STRING] = DataHolder::create(USBCommands::BRIDGE_SET_KILLSWITCH, false,
                                                         PRIORITY_KILL_SWITCH,
                                                         USBCommands::bridge::INACTIVE);
    }
}

bool bridge::Interface::isButtonPressed(Button button) {
    return buttons[button];
}

void bridge::Interface::MotorClass::SingleMotor::onKillSwitchActivated() {
    power = 0;
    programmedSpeed = 0;
    direction = Direction::STOP;
}

void bridge::Interface::MotorClass::SingleMotor::initStructure() {
    if (requests.find(getKey()) == requests.end()) {
        programmedSpeed = 0;
        direction = Direction::STOP;
        power = 0;
        createMotorState();
    }
}

void bridge::Interface::MotorClass::SingleMotor::createMotorState() {
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
        case Direction::FORWARD:
            mState.direction = motor::FORWARD;
            break;
        case Direction::BACKWARD:
            mState.direction = motor::BACKWARD;
            break;
        case Direction::STOP:
        default:
            mState.direction = motor::STOP;
            break;
    };

    requests[getKey()] = DataHolder::create(USBCommands::MOTOR_DRIVER_SET, PRIORITY_MOTOR_SET, true, mState);
}

void bridge::Interface::MotorClass::SingleMotor::setSpeed(unsigned int speed) {
    if (speed <= MOTOR_DRIVER_MAX_SPEED) {
        programmedSpeed = speed;
    } else {
        logger.warn("Trying to set greater speed for %s than max: %d.", devToString(motor).c_str(), speed);
        programmedSpeed = MOTOR_DRIVER_MAX_SPEED;
    }

    createMotorState();

    logger.info("Setting speed of %s to %d.", devToString(motor).c_str(), (int) programmedSpeed);
}

void bridge::Interface::MotorClass::SingleMotor::setDirection(Direction dir) {
    this->direction = dir;

    createMotorState();

    logger.info("Setting direction of %s to %s.", devToString(motor).c_str(), directionToString(dir).c_str());
}

void bridge::Interface::MotorClass::brake() {
    motors[Motor::LEFT]->setDirection(Direction::STOP);
    motors[Motor::RIGHT]->setDirection(Direction::STOP);

    logger.info("Braking all motors.");
}

void bridge::Interface::ArmClass::SingleJoint::onKillSwitchActivated() {
    direction = Direction::STOP;
    speed = 0;
}

void bridge::Interface::ArmClass::SingleJoint::initStructure() {
    if (requests.find(getKey()) == requests.end()) {
        direction = Direction::STOP;
        speed = 0;
        settingPosition = false;
        programmedPosition = 0;

        createJointState();
    }
}

void bridge::Interface::ArmClass::SingleJoint::createJointState() {
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

void bridge::Interface::ArmClass::SingleJoint::setSpeed(unsigned int speed) {
    if (calibrationStatus == ArmCalibrationStatus::IN_PROGRESS) {
        //todo make sure it's valid logic
        logger.info("Ignoring set speed for %s because calibration in process.", devToString(joint).c_str());
        return;
    }

    unsigned int effectiveSpeed = 0;
    if (speed <= ARM_DRIVER_MAX_SPEED) {
        effectiveSpeed = speed;
    } else {
        logger.warn("Trying to set greater speed for %s than max: %d.",
                    devToString(joint).c_str(), ARM_DRIVER_MAX_SPEED);
        effectiveSpeed = ARM_DRIVER_MAX_SPEED;
    }

    this->speed = speed;

    createJointState();

    logger.info("Setting speed of %s to %d.", devToString(joint).c_str(), (int) effectiveSpeed);
}

void bridge::Interface::ArmClass::SingleJoint::setDirection(Direction direction) {
    if (calibrationStatus == ArmCalibrationStatus::IN_PROGRESS) {
        //todo make sure it's valid logic
        logger.info("Ignoring set direction for %s because calibration in process.", devToString(joint).c_str());
        return;
    }

    this->direction = direction;
    this->settingPosition = false;
    //TODO ustawianie armDriverMode
    //mode = ArmDriverMode::DIRECTIONAL;

    createJointState();

    logger.info("Setting direction of %s to %s.", devToString(joint).c_str(), directionToString(direction).c_str());
}

void bridge::Interface::ArmClass::SingleJoint::setPosition(unsigned int position) {
    if (calibrationStatus == ArmCalibrationStatus::IN_PROGRESS) {
        //todo make sure it's valid logic
        logger.info("Ignoring set position for %s because calibration in process.", devToString(joint).c_str());
        return;
    }

    unsigned int effectivePos;
    if (position <= ARM_DRIVER_MAX_POSITION[joint]) {
        effectivePos = position;
    } else {
        logger.warn("Trying to set greater position for %s than max: %d.",
                    devToString(joint).c_str(), ARM_DRIVER_MAX_POSITION[joint]);
        effectivePos = ARM_DRIVER_MAX_POSITION[joint];
    }

    this->programmedPosition = effectivePos;
    this->settingPosition = true;
    //TODO ustawianie armDriverMode
    //mode = ArmDriverMode::POSITIONAL;

    createJointState();

    logger.info("Setting position of %s to %d.", devToString(joint).c_str(), (int) effectivePos);
}

void bridge::Interface::ArmClass::brake() {
    requests["arm_addon"] = DataHolder::create(USBCommands::ARM_DRIVER_SET, PRIORITY_ARM_GENERAL_SET, true,
                                               USBCommands::arm::BRAKE);

    logger.notice("Braking all joints.");
}

void bridge::Interface::ArmClass::calibrate() {
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

void bridge::Interface::ExpanderClass::Device::setEnabled(bool enabled) {
    if (enabled) {
        expanderByte |= (1 << int(device));
        logger.info(string("Enabling expander device ") + devToString(device) + ".");
    } else {
        expanderByte &= ~(1 << int(device));
        logger.info(string("Disabling expander device ") + devToString(device) + ".");
    }

    requests["expander"] = DataHolder::create(USBCommands::EXPANDER_SET, PRIORITY_EXPANDER_SET, true, expanderByte);
}

bool bridge::Interface::ExpanderClass::Device::isEnabled() {
    return ((1 << int(device)) & expanderByte);
}

void bridge::Interface::updateDataStructures(std::vector<USBCommands::Request> getterRequests,
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
                                      std::find(deviceResponse.begin() + actualPosition, deviceResponse.end(),
                                                USBCommands::MESSAGE_END));

        throw runtime_error(
                string("MESSAGE_END not found in the response at position: ") + to_string(actualPosition) + ", but: "
                + to_string(foundPos));
    }
}

unsigned int bridge::Interface::ExpanderClass::updateFields(USBCommands::Request request, uint8_t *data) {
    if (request != USBCommands::EXPANDER_GET) {
        return 0;
    }

    expanderByte = data[0];

    logger.info("Updating state expander byte.");

    return 1;
}

unsigned int bridge::Interface::ArmClass::updateFields(USBCommands::Request request, uint8_t *data) {
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

unsigned int bridge::Interface::MotorClass::SingleMotor::updateFields(USBCommands::Request request, uint8_t *data) {
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

    logger.info("Updating state motor %s power: %d.", devToString(motorNo).c_str(), (int) power);

    return sizeof(USBCommands::motor::SpecificMotorState);
}

unsigned int bridge::Interface::ArmClass::SingleJoint::updateFields(USBCommands::Request request, uint8_t *data) {
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

    logger.info("Updating state joint %s: direction: %s, position: %d.",
                devToString(jointNo).c_str(), directionToString(direction).c_str(), (int) position);

    return sizeof(USBCommands::arm::JointState);
}

unsigned int bridge::Interface::updateFields(USBCommands::Request request, uint8_t *data) {
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

    rawCurrent.push_back(curr);
    rawVoltage.push_back(volt);

    logger.debug("Raw voltage: %u, raw current: %u.", volt, curr);

    logger.info("Updating state: kill switch: %d (by hardware: %d), battery: %.1fV, %.1fA.",
                killSwitchActive, killSwitchCausedByHardware, getVoltage(), getCurrent());

    return sizeof(USBCommands::bridge::State);
}

void bridge::Interface::ArmClass::onKillSwitchActivated() {
    mode = ArmDriverMode::DIRECTIONAL;

    if (calibrationStatus == ArmCalibrationStatus::IN_PROGRESS) {
        calibrationStatus = ArmCalibrationStatus::NONE;
    }

    requests.erase("arm_addon");
}

void bridge::Interface::ExpanderClass::onKillSwitchActivated() {
}

void bridge::Interface::updateStructsWhenKillSwitchActivated() {
    for (auto listener : extDevListeners) {
        listener->onKillSwitchActivated();
    }
}

void bridge::Interface::onKillSwitchActivated() {
    lcdText = "";
}

