#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Category.hh>

#include "ColorPatternLayout.hpp"
#include "logging.hpp"

void common::logger::configureLogger(std::string &propertiesFile,
		log4cpp::Priority::PriorityLevel priority,
		bool enableColor) {

	if (propertiesFile.length() != 0) {
		log4cpp::PropertyConfigurator::configure(propertiesFile);
	}

	log4cpp::PatternLayout *patternLayout = enableColor ? new ColorPatternLayout() : new log4cpp::PatternLayout();
	patternLayout->setConversionPattern("%d{%H:%M:%S.%l} [%p] %c: %m%n");

	log4cpp::Category &category = log4cpp::Category::getRoot();
	category.setPriority(priority);
	category.getAppender()->setLayout(patternLayout);
}
