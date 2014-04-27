/*
 * BridgeProcessor.hpp
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
#ifndef BRIDGEPROCESSOR_HPP_
#define BRIDGEPROCESSOR_HPP_

#include <memory>
#include <thread>
#include <mutex>
#include <chrono>

#include <log4cpp/Category.hh>
#include <wallaroo/device.h>
#include <json/value.h>

#include "IRequestProcessor.hpp"
#include "USBCommunicator.hpp"
#include "Interface.hpp"

namespace bridge {

class BridgeProcessor: public processing::IRequestProcessor, public wallaroo::Device {
public:
	BridgeProcessor();
	~BridgeProcessor();

	virtual Json::Value process(Json::Value request) override;

private:
	log4cpp::Category& logger;

	// shared data
	wallaroo::Plug<ICommunicator> usbComm;
	Interface iface;

	std::unique_ptr<std::thread> maintenanceThread;
	std::mutex maintenanceMutex;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastProcessFunctionExecution;

	volatile bool finishCycleThread = false;

	void maintenanceThreadFunction();
};

} /* namespace bridge */

#endif /* BRIDGEPROCESSOR_HPP_ */
