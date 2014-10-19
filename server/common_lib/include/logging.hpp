#ifndef _LOGGING_HPP_
#define _LOGGING_HPP_

#include <string>
#include <vector>

#include <log4cpp/Priority.hh>

namespace common {
	namespace logger {
		void configureLogger(std::vector<std::string> &propertiesFiles, std::string priority, bool enableColor);

		void configureLogger(std::string &propertiesFile, std::string priority, bool enableColor);
	}
}

#endif