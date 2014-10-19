#ifndef _LOGGING_HPP_
#define _LOGGING_HPP_

#include <string>

#include <log4cpp/Priority.hh>

namespace common {
	namespace logger {
		void configureLogger(std::string &propertiesFile,
				log4cpp::Priority::PriorityLevel priority,
				bool enableColor);
	}
}

#endif