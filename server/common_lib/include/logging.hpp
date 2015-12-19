#pragma once

#include <log4cpp/Priority.hh>

#include <string>
#include <vector>

namespace common {
    namespace logger {
        void configureLogger(std::vector<std::string> &propertiesFiles, std::string priority, bool enableColor);

        void configureLogger(std::string &propertiesFile, std::string priority, bool enableColor);
    }
}
