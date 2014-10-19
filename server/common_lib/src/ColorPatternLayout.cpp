#include <map>

#include <cstdio>
#include <unistd.h>

#include "ColorPatternLayout.hpp"

#define FG_BLACK "\x1b[0;30m"
#define FG_RED "\x1b[0;31m"
#define FG_GREEN "\x1b[0;32m"
#define FG_BROWN "\x1b[0;33m"
#define FG_BLUE "\x1b[0;34m"
#define FG_PURPLE "\x1b[0;35m"
#define FG_CYAN "\x1b[0;36m"
#define FG_LIGHT_GRAY "\x1b[0;37m"
#define FG_DARK_GRAY "\x1b[1;30m"
#define FG_LIGHT_RED "\x1b[1;31m"
#define FG_LIGHT_GREEN "\x1b[1;32m"
#define FG_YELLOW "\x1b[1;33m"
#define FG_LIGHT_BLUE "\x1b[1;34m"
#define FG_LIGHT_PURPLE "\x1b[1;35m"
#define FG_LIGHT_CYAN "\x1b[1;36m"
#define FG_WHITE "\x1b[1;37m"

#define FG_DEFAULT "\x1b[0m"

static std::map<std::string, std::string> colorMap = {
		{"black", FG_BLACK},
		{"red", FG_RED},
		{"green", FG_GREEN},
		{"brown", FG_BROWN},
		{"blue", FG_BLUE},
		{"purple", FG_PURPLE},
		{"cyan", FG_CYAN},
		{"light-gray", FG_LIGHT_GRAY},
		{"dark-gray", FG_DARK_GRAY},
		{"light-red", FG_LIGHT_RED},
		{"light-green", FG_LIGHT_GREEN},
		{"yellow", FG_YELLOW},
		{"light-blue", FG_LIGHT_BLUE},
		{"light-purple", FG_LIGHT_PURPLE},
		{"light-cyan", FG_LIGHT_CYAN},
		{"white", FG_WHITE}
};

common::logger::ColorPatternLayout::ColorPatternLayout() {
	priorityColors[log4cpp::Priority::DEBUG] = colorMap["light-gray"];
	priorityColors[log4cpp::Priority::INFO] = colorMap["light-gray"];
	priorityColors[log4cpp::Priority::NOTICE] = colorMap["green"];
	priorityColors[log4cpp::Priority::WARN] = colorMap["yellow"];
	priorityColors[log4cpp::Priority::ERROR] = colorMap["light-red"];
	//TODO read this from file

	if (!isatty(fileno(stdout))) {
		//todo warning colors are disabled
		enableColors = false;
	}
}

common::logger::ColorPatternLayout::~ColorPatternLayout() {
}

std::string common::logger::ColorPatternLayout::format(const log4cpp::LoggingEvent &event) {
	if (enableColors) {
		return priorityColors.at(static_cast<log4cpp::Priority::PriorityLevel>(event.priority))
				+ log4cpp::PatternLayout::format(event)
				+ FG_DEFAULT;
	} else {
		return log4cpp::PatternLayout::format(event);
	}
}
