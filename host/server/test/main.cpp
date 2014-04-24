/*
 * main.cpp
 *
 *  Project: server
 *  Created on: 6 kwi 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE SZARKServerTest
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <log4cpp/PropertyConfigurator.hh>
#include <boost/test/unit_test.hpp>

#include "Configuration.hpp"

class AllocatorSetup {
public:
	AllocatorSetup();
	~AllocatorSetup();
};

BOOST_GLOBAL_FIXTURE(AllocatorSetup);

AllocatorSetup::AllocatorSetup() {
	std::string initFileName = "loggerTest.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);
	config::Configuration::create();
}

AllocatorSetup::~AllocatorSetup() {

}
