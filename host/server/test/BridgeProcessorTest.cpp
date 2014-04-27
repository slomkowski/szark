/*
 * BridgeProcessorTest.cpp
 *
 *  Project: server
 *  Created on: 26 kwi 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include <thread>
#include <chrono>

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "BridgeProcessor.hpp"

BOOST_AUTO_TEST_CASE(BridgeProcessorTest_Run) {
	wallaroo::Catalog catalog;

	catalog.Create("comm", "USBCommunicator");
	catalog.Create("bp", "BridgeProcessor");

	wallaroo::use(catalog["comm"]).as("communicator").of(catalog["bp"]);

	catalog.CheckWiring();

	std::shared_ptr<bridge::BridgeProcessor> proc = catalog["bp"];

	std::this_thread::sleep_for(std::chrono::seconds(1));

	BOOST_CHECK_EQUAL(1, 1);
}

