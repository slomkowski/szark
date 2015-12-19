#include "convert.hpp"

#include <boost/algorithm/string.hpp>

using namespace common::bridge;

std::string common::bridge::devToString(ExpanderDevice dev) {
    switch (dev) {
        case ExpanderDevice::LIGHT_CAMERA:
            return "camera light";
        case ExpanderDevice::LIGHT_LEFT:
            return "left light";
        default:
            return "right light";
    };
}

std::string common::bridge::devToString(Joint dev) {
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

std::string common::bridge::devToString(Motor dev) {
    if (dev == Motor::LEFT) {
        return "left";
    } else {
        return "right";
    }
}


std::string common::bridge::directionToString(const Direction dir) {
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

Direction common::bridge::stringToDirection(std::string dir) {
    boost::algorithm::to_lower(dir);

    if (dir == "stop") {
        return Direction::STOP;
    } else if (dir == "forward") {
        return Direction::FORWARD;
    } else if (dir == "backward") {
        return Direction::BACKWARD;
    } else {
        throw std::runtime_error("invalid direction: " + dir);
    }

    return Direction::STOP;
}

std::string common::bridge::armDriverModeToString(const ArmDriverMode mode) {
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

std::string common::bridge::armCalibrationStatusToString(const ArmCalibrationStatus status) {
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
