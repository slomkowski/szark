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
#include "Interface.hpp"

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

void BridgeProcessor::process(Json::Value& request, Json::Value& response) {
	// TODO process in bridgeprocessor

	//TODO wyślij do interfejsu dane z jsona

	iface.syncWithDevice([&](vector<uint8_t> r) {
		usbComm->sendData(r);
		return usbComm->receiveData();
	});

	createReport(response);

	lastProcessFunctionExecution = high_resolution_clock::now();
}

void BridgeProcessor::maintenanceThreadFunction() {
	while (true) {
		this_thread::sleep_for(MAINTAINANCE_TASK_INTERVAL);

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

		iface.syncWithDevice([&](vector<uint8_t> r) {
			usbComm->sendData(r);
			return usbComm->receiveData();
		});

		lk.unlock();
	}
}

void BridgeProcessor::createReport(Json::Value& r) {
	using bridge::ExpanderDevice;
	using bridge::Motor;
	using bridge::Joint;
	using bridge::Button;

	//TODO stwórz raport jsonowy

	auto fillExpander = [&](string name, ExpanderDevice d) {
		r["lights"][name] = iface.expander[d].isEnabled();
	};

	auto fillButtons = [&](string name, Button d) {
		if(iface.isButtonPressed(d)) {
			r["buttons"].append(name);
		}
	};

	auto fillMotor = [&](string name, Motor m) {
		r["motors"][name]["speed"] = iface.motor[m].getSpeed();
		r["motors"][name]["dir"] = directionToString(iface.motor[m].getDirection());
	};

	auto fillArm = [&](string name, Joint j) {
		r["arms"][name]["speed"] = iface.arm[j].getSpeed();
		r["arms"][name]["pos"] = iface.arm[j].getPosition();
		r["arms"][name]["dir"] = directionToString(iface.arm[j].getDirection());
	};

	fillExpander("right", ExpanderDevice::LIGHT_RIGHT);
	fillExpander("left", ExpanderDevice::LIGHT_LEFT);
	fillExpander("camera", ExpanderDevice::LIGHT_CAMERA);

	fillMotor("left", Motor::LEFT);
	fillMotor("right", Motor::RIGHT);

	fillArm("shoulder", Joint::SHOULDER);
	fillArm("elbow", Joint::ELBOW);
	fillArm("wrist", Joint::WRIST);
	fillArm("gripper", Joint::GRIPPER);

	fillButtons("up", Button::UP);
	fillButtons("down", Button::DOWN);
	fillButtons("enter", Button::ENTER);

	r["batt"]["volt"] = iface.getVoltage();
	r["batt"]["curr"] = iface.getCurrent();

	// TODO killswitch
}

}
