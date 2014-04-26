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

#include <memory>
#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "RequestQueuer.hpp"
#include "IRequestProcessor.hpp"

class RequestProcessorMock: public processing::IRequestProcessor, public wallaroo::Device {
public:
	virtual Json::Value process(Json::Value request);
};

Json::Value RequestProcessorMock::process(Json::Value request) {
	Json::Value response;

	response["serial"] = request["serial"];

	response["lights"]["led"] = false;
	response["lights"]["camera"] = true;

	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	return response;
}

WALLAROO_REGISTER(RequestProcessorMock);

std::string requestWithNoSerial = R"(
{
     "timestamp" : "2014-01-01 12:34:56,123",
     "control" : "stop"
}
)";

std::string validRequest =
		R"(
{
     "serial" : 55555555,
     "timestamp" : "2014-01-01 12:34:56,123",
     "control" : "stop",
     "motors" :
     {
	  "left" : { "speed" : 12, "direction" : "forward" },
	  "right" : { "speed" : 12, "direction" : "forward" }
     },
     "arms" :
     {
	  "wrist" : { "speed" : 12, "position" : 45 },
	  "gripper" : { "speed" : 12, "direction" : "backward" },
	  "elbow" : { "speed" : 12, "direction" : "backward" },
	  "shoulder" : { "speed" : 12, "direction" : "stop" }
     },
     "lights" :
     {
	  "led" : true,
	  "camera" : false
     }
}
)";

std::string validRequestWithLowerSerial =
		R"(
{
     "serial" : 11111111,
     "timestamp" : "2014-01-01 12:34:56,123",
     "control" : "stop",
     "motors" :
     {
	  "left" : { "speed" : 12, "direction" : "forward" },
	  "right" : { "speed" : 12, "direction" : "forward" }
     }
})";

std::string validRequestWithZeroSerial =
		R"(
{
     "serial" : 0,
     "timestamp" : "2014-01-01 12:34:56,123",
     "control" : "stop"
})";

std::string validRequestWithHigherSerial =
		R"(
{
     "serial" : 55555556,
     "timestamp" : "2014-01-01 12:34:56,123",
     "control" : "stop",
     "motors" :
     {
	  "left" : { "speed" : 12, "direction" : "forward" },
	  "right" : { "speed" : 12, "direction" : "forward" }
     }
})";

BOOST_AUTO_TEST_CASE(RequestQueuerTest_addRequest) {
	wallaroo::Catalog catalog;

	catalog.Create("rq", "RequestQueuer");
	catalog.Create("mock1", "RequestProcessorMock");
	wallaroo::use(catalog["mock1"]).as("requestProcessors").of(catalog["rq"]);

	catalog.CheckWiring();

	std::shared_ptr<processing::RequestQueuer> rq = catalog["rq"];

	BOOST_CHECK_EQUAL(rq->getNumOfProcessors(), 1);

	std::pair<std::string, bool> tests[] = {
			std::make_pair("invalid string", false),
			std::make_pair(validRequest, true),
			std::make_pair(requestWithNoSerial, false),
			std::make_pair(validRequest, false),
			std::make_pair(validRequestWithLowerSerial, false),
			std::make_pair(validRequestWithHigherSerial, true),
			std::make_pair(validRequestWithHigherSerial, false),

			std::make_pair(validRequestWithZeroSerial, true),
			std::make_pair(validRequestWithZeroSerial, true),
			std::make_pair(validRequestWithZeroSerial, true),

			std::make_pair(requestWithNoSerial, false),
			std::make_pair(validRequest, true),
			std::make_pair(validRequestWithLowerSerial, false),
			std::make_pair(validRequestWithHigherSerial, true),
			std::make_pair(validRequestWithHigherSerial, false)
	};

	for (auto t : tests) {
		BOOST_CHECK_EQUAL(rq->addRequest(t.first), t.second);
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}

	catalog.Create("mock2", "RequestProcessorMock");
	wallaroo::use(catalog["mock2"]).as("requestProcessors").of(catalog["rq"]);
	catalog.CheckWiring();

	BOOST_CHECK_EQUAL(rq->getNumOfProcessors(), 2);
}

