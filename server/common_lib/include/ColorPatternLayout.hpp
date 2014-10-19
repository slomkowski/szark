#ifndef _COLORPATTERNLAYOUT_HPP_
#define  _COLORPATTERNLAYOUT_HPP_

#include <map>
#include <string>
#include <log4cpp/PatternLayout.hh>

namespace common {
	namespace logger {
		class ColorPatternLayout : public log4cpp::PatternLayout {
		public:
			ColorPatternLayout();

			virtual ~ColorPatternLayout();

			virtual std::string format(const log4cpp::LoggingEvent &event);

		private:
			std::map<log4cpp::Priority::PriorityLevel, std::string> priorityColors;

			bool enableColors = true;
		};
	}
}

#endif