/*
 * ConfigurationTest.cpp
 *
 *  Project: server
 *  Created on: 24 kwi 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include "Configuration.hpp"

#include <boost/test/unit_test.hpp>

using namespace common;

BOOST_AUTO_TEST_CASE(ConfigurationTest_Run) {
    auto config = new config::Configuration();

    BOOST_CHECK_THROW(config->getInt("invalid_int"), config::ConfigException);
    BOOST_CHECK_THROW(config->getBool("invalid_bool"), config::ConfigException);
    BOOST_CHECK_THROW(config->getString("invalid_string"), config::ConfigException);

    config->putString("test.string", "qwerty");
    BOOST_CHECK_EQUAL(config->getString("test.string"), "qwerty");

    config->putBool("test.bool", true);
    BOOST_CHECK_EQUAL(config->getBool("test.bool"), true);

    config->putInt("test.int", 123);
    BOOST_CHECK_EQUAL(config->getInt("test.int"), 123);

//    int port = config->getInt("szark.server.NetServer.port");
//
//    BOOST_TEST_MESSAGE(std::string("port: ") + std::to_string(port));

    // TODO zrobić unit testa

    delete config;
}

