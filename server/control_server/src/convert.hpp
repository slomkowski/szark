#ifndef _CONVERT_HPP_
#define _CONVERT_HPP_

#include "Interface.hpp"

namespace bridge {
	namespace convert {
		/**
		* Converts Direction enum to string (lowercase letters).
		* @param dir
		* @return
		*/
		std::string directionToString(const Direction dir);

		/**
		* Converts string to direction enum. Throws runtime_error if invalid string.
		* @param dir
		* @return
		*/
		Direction stringToDirection(std::string dir);

		std::string armDriverModeToString(const ArmDriverMode mode);

		std::string armCalibrationStatusToString(const ArmCalibrationStatus status);

		std::string devToString(ExpanderDevice dev);

		std::string devToString(Joint dev);

		std::string devToString(Motor dev);
	}
}

#endif