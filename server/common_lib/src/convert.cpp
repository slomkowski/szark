#include "convert.hpp"

#include <boost/algorithm/string.hpp>

using namespace common::bridge;

const std::string &common::bridge::devToString(ExpanderDevice dev) {
    static const std::string text[] = {
            "camera light",
            "left light",
            "right light"
    };
    switch (dev) {
        case ExpanderDevice::LIGHT_CAMERA:
            return text[0];
        case ExpanderDevice::LIGHT_LEFT:
            return text[1];
        default:
            return text[2];
    };
}

const std::string &common::bridge::devToString(Joint dev) {
    static const std::string text[] = {
            "elbow",
            "gripper",
            "shoulder"
    };

    switch (dev) {
        case Joint::ELBOW:
            return text[0];
        case Joint::GRIPPER:
            return text[1];
        case Joint::SHOULDER:
        default:
            return text[2];
    };
}

const std::string &common::bridge::devToString(Motor dev) {
    static const std::string text[] = {
            "left",
            "right"
    };

    if (dev == Motor::LEFT) {
        return text[0];
    } else {
        return text[1];
    }
}

const std::string &common::bridge::directionToString(const Direction dir) {
    static const std::string text[] = {
            "forward",
            "backward",
            "stop"
    };

    switch (dir) {
        case Direction::FORWARD:
            return text[0];
        case Direction::BACKWARD:
            return text[1];
        case Direction::STOP:
        default:
            return text[2];
    }
}

const Direction common::bridge::stringToDirection(const std::string &dirc) {
    std::string dir = dirc;
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

const std::string &common::bridge::armDriverModeToString(const ArmDriverMode mode) {
    static const std::string text[] = {
            "calibrating",
            "positional",
            "directional"
    };

    switch (mode) {
        case ArmDriverMode::CALIBRATING:
            return text[0];
        case ArmDriverMode::POSITIONAL:
            return text[1];
        case ArmDriverMode::DIRECTIONAL:
        default:
            return text[2];
    }
}

const std::string &common::bridge::armCalibrationStatusToString(const ArmCalibrationStatus status) {
    static const std::string text[] = {
            "done",
            "prog",
            "none"
    };
    switch (status) {
        case ArmCalibrationStatus::DONE:
            return text[0];
        case ArmCalibrationStatus::IN_PROGRESS:
            return text[1];
        case ArmCalibrationStatus::NONE:
        default:
            return text[2];
    }
}
