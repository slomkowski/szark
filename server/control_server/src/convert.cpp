#include <boost/algorithm/string.hpp>

#include "convert.hpp"

using namespace bridge;

std::string bridge::convert::devToString(ExpanderDevice dev) {
	switch (dev) {
		case ExpanderDevice::LIGHT_CAMERA:
			return "camera light";
		case ExpanderDevice::LIGHT_LEFT:
			return "left light";
		default:
			return "right light";
	};
}

std::string bridge::convert::devToString(Joint dev) {
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

std::string bridge::convert::devToString(Motor dev) {
	if (dev == Motor::LEFT) {
		return "left";
	} else {
		return "right";
	}
}


std::string bridge::convert::directionToString(const Direction dir) {
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

Direction bridge::convert::stringToDirection(std::string dir) {
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

std::string bridge::convert::armDriverModeToString(const ArmDriverMode mode) {
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

std::string bridge::convert::armCalibrationStatusToString(const ArmCalibrationStatus status) {
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
