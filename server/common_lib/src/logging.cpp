#include "ColorPatternLayout.hpp"
#include "Configuration.hpp"
#include "logging.hpp"

#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>


static std::map<std::string, log4cpp::Priority::PriorityLevel> priorities = {
        {"debug",  log4cpp::Priority::DEBUG},
        {"info",   log4cpp::Priority::INFO},
        {"notice", log4cpp::Priority::NOTICE},
        {"warn",   log4cpp::Priority::WARN},
        {"error",  log4cpp::Priority::ERROR}
};

void common::logger::configureLogger(
        std::vector<std::string> &propertiesFiles, std::string priority, bool enableColor) {

    log4cpp::Category &category = log4cpp::Category::getRoot();
    category.setAppender(new log4cpp::OstreamAppender("console", &std::cout));

    log4cpp::PatternLayout *patternLayout = enableColor ? new ColorPatternLayout() : new log4cpp::PatternLayout();
    patternLayout->setConversionPattern("%d{%H:%M:%S.%l} [%p] %c: %m%n");
    category.getAppender()->setLayout(patternLayout);


    for (auto &fileName : propertiesFiles) {
        using namespace boost::property_tree;

        ptree ptree;
        read_ini(fileName, ptree);

        std::string className;
        std::string logLevel;

        for (auto &child : ptree) {
            try {
                className = child.first;
                logLevel = child.second.get<std::string>("loglevel");

                boost::algorithm::to_lower(logLevel);
                log4cpp::Category::getInstance(className).setPriority(priorities.at(logLevel));
            } catch (ptree_bad_path) {
            } catch (std::out_of_range) {
                throw std::runtime_error(std::string("invalid configuration file log level argument: " + logLevel));
            }
        }
    }

    try {
        boost::algorithm::to_lower(priority);
        category.setPriority(priorities.at(priority));
    } catch (std::out_of_range) {
        throw std::runtime_error(std::string("invalid command line log level argument: " + priority));
    }
}

void ::common::logger::configureLogger(std::string &propertiesFile, std::string priority, bool enableColor) {
    std::vector<std::string> v = {propertiesFile};
    configureLogger(v, priority, enableColor);
}
