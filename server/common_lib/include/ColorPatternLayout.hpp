#pragma once

#include <log4cpp/PatternLayout.hh>

#include <map>
#include <string>

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
