#define BOOST_TEST_MODULE SZARKCommonTest
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include "Configuration.hpp"

#include <log4cpp/PropertyConfigurator.hh>
#include <boost/test/unit_test.hpp>

class AllocatorSetup {
public:
    AllocatorSetup();

    ~AllocatorSetup();
};

BOOST_GLOBAL_FIXTURE(AllocatorSetup);

AllocatorSetup::AllocatorSetup() {
    std::string initFileName = "loggerTest.properties";
    log4cpp::PropertyConfigurator::configure(initFileName);
}

AllocatorSetup::~AllocatorSetup() {
}
