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
#include <functional>

#include <log4cpp/Category.hh>
#include <wallaroo/device.h>
#include <json/value.h>

#include "IRequestProcessor.hpp"
#include "USBCommunicator.hpp"
#include "InterfaceManager.hpp"

namespace bridge {

class BridgeProcessor: public processing::IRequestProcessor, public wallaroo::Device {
public:
	BridgeProcessor();
	~BridgeProcessor();

	virtual void process(Json::Value& request, Json::Value& response) override;

private:
	log4cpp::Category& logger;

	// shared data
	wallaroo::Plug<ICommunicator> usbComm;
	InterfaceManager iface;

	std::unique_ptr<std::thread> maintenanceThread;
	std::mutex maintenanceMutex;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastProcessFunctionExecution;

	volatile bool finishCycleThread = false;

	void maintenanceThreadFunction();

	void createReport(Json::Value& r);
	void parseRequest(Json::Value& r);

	template<typename T> void tryAssign(const Json::Value& key, std::function<void(T)> setter);

	void fillAllDevices(
			std::function<void(std::string name, Joint j)> fillArm,
			std::function<void(std::string name, Motor m)> fillMotor,
			std::function<void(std::string name, ExpanderDevice d)> fillExpander);
};

} /* namespace bridge */

#endif /* BRIDGEPROCESSOR_HPP_ */
