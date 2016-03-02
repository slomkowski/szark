#pragma once

#include "Interface.hpp"

namespace common {
    namespace bridge {
        /**
        * Converts Direction enum to string (lowercase letters).
        * @param dir
        * @return
        */
        const std::string &directionToString(const Direction dir);

        /**
        * Converts string to direction enum. Throws runtime_error if invalid string.
        * @param dir
        * @return
        */
        const Direction stringToDirection(const std::string &dir);

        const std::string &armDriverModeToString(const ArmDriverMode mode);

        const std::string &armCalibrationStatusToString(const ArmCalibrationStatus status);

        const std::string &devToString(ExpanderDevice dev);

        const std::string &devToString(Joint dev);

        const std::string &devToString(Motor dev);
    }
}
