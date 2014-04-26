/*

 * WifiInfoTest.cpp
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

#include <boost/test/unit_test.hpp>

#include <unistd.h>

#include "WifiInfo.hpp"

BOOST_AUTO_TEST_CASE(WifiInfoTest_Run) {
	const std::string INTERFACE_NAME = "wlan0";

	BOOST_CHECK_THROW(os::WifiInfo("invalid_iface"), os::WifiException);

	os::WifiInfo wifi(INTERFACE_NAME);

	BOOST_CHECK_EQUAL(INTERFACE_NAME, wifi.getInterfaceName());

	for (int i = 0; i < 30; i++) {
		auto bitrate = wifi.getBitrate();
		auto sigLevel = wifi.getSignalLevel();

		BOOST_CHECK_GT(bitrate, 1.0);
		BOOST_CHECK_LT(bitrate, 300.0);

		BOOST_TEST_MESSAGE(std::string("wifi bitrate: ") + std::to_string(bitrate));

		BOOST_CHECK_GT(sigLevel, -150.0);
		BOOST_CHECK_LT(sigLevel, 0);

		BOOST_TEST_MESSAGE(std::string("wifi signal strength: ") + std::to_string(sigLevel));

		usleep(100000);
	}
}
