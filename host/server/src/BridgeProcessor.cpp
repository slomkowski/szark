/*
 * BridgeProcessor.cpp
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

#include <chrono>

#include <boost/format.hpp>

#include "BridgeProcessor.hpp"

using namespace std;
using boost::format;
using std::chrono::high_resolution_clock;

namespace bridge {

/**
 * If the process() function in the given timeout, the maintainance thread starts operation.
 * It prevents the device from going to stopped state, queries for battery etc.
 */
const chrono::milliseconds TIMEOUT(25);
const chrono::milliseconds MAINTAINANCE_TASK_INTERVAL(25);

WALLAROO_REGISTER(BridgeProcessor);

BridgeProcessor::BridgeProcessor()
		: logger(log4cpp::Category::getInstance("BridgeProcessor")),
				usbComm("communicator", RegistrationToken()),
				lastProcessFunctionExecution(high_resolution_clock::now()) {

	maintenanceThread.reset(new thread(&BridgeProcessor::maintenanceThreadFunction, this));

	logger.notice("Instance created.");
}

BridgeProcessor::~BridgeProcessor() {
	maintenanceMutex.lock();
	finishCycleThread = true;
	maintenanceMutex.unlock();

	logger.notice("Waiting for maintenance task to stop.");

	maintenanceThread->join();

	logger.notice("Instance destroyed.");
}

Json::Value BridgeProcessor::process(Json::Value request) {
	// TODO process in bridgeprocessor

	lastProcessFunctionExecution = high_resolution_clock::now();

	return request;
}

void BridgeProcessor::maintenanceThreadFunction() {
	while (true) {
		unique_lock<mutex> lk(maintenanceMutex);

		if (finishCycleThread) {
			break;
		}

		if ((not usbComm.WiringOk()) or ((high_resolution_clock::now() - lastProcessFunctionExecution) < TIMEOUT)) {
			lk.unlock();
			this_thread::yield();
			continue;
		}

		logger.info("Performing maintenance task.");

		// TODO tu standardowe zbieranie
		vector<uint8_t> dd;
		usbComm->sendData(dd);

		usbComm->receiveData();

		lk.unlock();

		this_thread::sleep_for(MAINTAINANCE_TASK_INTERVAL);
	}
}

} /* namespace bridge */
