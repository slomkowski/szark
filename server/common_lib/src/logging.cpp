#include <map>

#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Category.hh>
#include "ColorPatternLayout.hpp"
#include "logging.hpp"

static std::map<std::string, log4cpp::Priority::PriorityLevel> priorities = {
		{"debug", log4cpp::Priority::DEBUG},
		{"info", log4cpp::Priority::INFO},
		{"notice", log4cpp::Priority::NOTICE},
		{"warn", log4cpp::Priority::WARN},
		{"error", log4cpp::Priority::ERROR}
};

void common::logger::configureLogger(std::vector<std::string> &propertiesFiles, std::string priority, bool enableColor) {

	log4cpp::Category &category = log4cpp::Category::getRoot();

	for (auto &fileName : propertiesFiles) {
		log4cpp::PropertyConfigurator::configure(fileName);
	}

	log4cpp::PatternLayout *patternLayout = enableColor ? new ColorPatternLayout() : new log4cpp::PatternLayout();
	patternLayout->setConversionPattern("%d{%H:%M:%S.%l} [%p] %c: %m%n");

	// TODO category.setPriority(priority);
	category.getAppender()->setLayout(patternLayout);
}

void ::common::logger::configureLogger(std::string &propertiesFile, std::string priority, bool enableColor) {
	std::vector<std::string> v = {propertiesFile};
	configureLogger(v, priority, enableColor);
}
