/*
 * main.cpp
 *
 *  Created on: 02-08-2013
 *      Author: michal
 */

#include <iostream>
#include <boost/timer.hpp>
#include <chrono>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <wallaroo/catalog.h>

#include "Configuration.hpp"
#include "logging.hpp"
#include "NetServer.hpp"

using namespace std;
using namespace wallaroo;

int main(int argc, char *argv[]) {
	std::string initFileName = "logger.properties";

	common::logger::configureLogger(initFileName, log4cpp::Priority::DEBUG, true);

	Catalog c;
	c.Create("conf", "Configuration", initFileName);
	c.Create("comm", "USBCommunicator");
	c.Create("netServer", "NetServer");
	c.Create("reqQueuer", "RequestQueuer");
	c.Create("bridgeProc", "BridgeProcessor");

	wallaroo_within(c) {
		use("conf").as("config").of("netServer");
		use("comm").as("communicator").of("bridgeProc");
		use("reqQueuer").as("requestQueuer").of("netServer");
		use("bridgeProc").as("requestProcessors").of("reqQueuer");
		// TODO napisać i dodać os processor - min. wifi
	}

	c.CheckWiring();
	c.Init();

	auto netServer = shared_ptr<processing::INetServer>(c["netServer"]);

	netServer->run();
}

