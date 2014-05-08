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

#include <log4cpp/PropertyConfigurator.hh>
#include <wallaroo/catalog.h>

#include "Configuration.hpp"
#include "NetServer.hpp"

using namespace std;
using namespace wallaroo;

int main(int argc, char *argv[]) {
	std::string initFileName = "logger.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);

	config::Configuration::create();

	Catalog c;
	c.Create("comm", "USBCommunicator");
	c.Create("netServer", "NetServer");
	c.Create("reqQueuer", "RequestQueuer");
	c.Create("bridgeProc", "BridgeProcessor");

	use(c["comm"]).as("communicator").of(c["bridgeProc"]);
	use(c["reqQueuer"]).as("requestQueuer").of(c["netServer"]);
	use(c["bridgeProc"]).as("requestProcessors").of(c["reqQueuer"]);

	c.CheckWiring();

	auto netServer = shared_ptr<processing::INetServer>(c["netServer"]);

	netServer->run();
}

