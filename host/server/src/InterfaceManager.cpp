/*
 * InterfaceManager.cpp
 *
 *  Project: server
 *  Created on: 24-08-2013
 *
 *  Copyright 2013 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include "InterfaceManager.hpp"
#include "USBCommunicator.hpp"

#include <cstring>
#include <thread>
#include <chrono>

using namespace std;

namespace bridge {

WALLAROO_REGISTER(InterfaceManager)

InterfaceManager::InterfaceManager()
		: logger(log4cpp::Category::getInstance("InterfaceManager")), counter(0), usbComm("ICommunicator",
				RegistrationToken()) {
}

InterfaceManager::~InterfaceManager() {
	// TODO Auto-generated destructor stub
}

vector<uint8_t> InterfaceManager::generateGetRequests(bool killSwitchActive) {
	vector<uint8_t> requests;
	getterRequests.clear();

	requests.push_back(USBCommands::Request::BRIDGE_GET_STATE);
	getterRequests.push_back(USBCommands::Request::BRIDGE_GET_STATE);

	// if the kill switch is active, devices are in reset state and won't repond anyway
	if (killSwitchActive) {
		return requests;
	}

	requests.push_back(USBCommands::Request::MOTOR_DRIVER_GET);
	getterRequests.push_back(USBCommands::Request::MOTOR_DRIVER_GET);
	if (counter % 2) {
		requests.push_back(motor::MOTOR1);
	} else {
		requests.push_back(motor::MOTOR2);
	}

	requests.push_back(USBCommands::Request::ARM_DRIVER_GET);
	getterRequests.push_back(USBCommands::Request::ARM_DRIVER_GET);

	switch (counter % 5) {
	case 0:
		requests.push_back(arm::ELBOW);
		break;
	case 1:
		requests.push_back(arm::WRIST);
		break;
	case 2:
		requests.push_back(arm::GRIPPER);
		break;
	case 3:
		requests.push_back(arm::SHOULDER);
		break;
	case 4:
		requests.pop_back();
		requests.push_back(USBCommands::Request::ARM_DRIVER_GET_GENERAL_STATE);
		getterRequests.pop_back();
		getterRequests.push_back(USBCommands::Request::ARM_DRIVER_GET_GENERAL_STATE);
		break;
	};

	counter++;

	return requests;
}

void InterfaceManager::stageChanges() {

	vector<uint8_t> concatenated;

	auto diff = generateDifferentialRequests();

	// ensure that disabling kill switch is the first command
	auto killSwitchRequest = diff.find(KILLSWITCH_STRING);
	if (killSwitchRequest != diff.end()
			and killSwitchRequest->second->getPlainData()[1] == USBCommands::bridge::INACTIVE) {
		logger.notice("disabling kill switch");
		killSwitchRequest->second->appendTo(concatenated);
	}

	for (auto& request : diff) {
		if (request.first == KILLSWITCH_STRING) {
			continue;
		}

		request.second->appendTo(concatenated);
	}

	// append getter requests
	auto getterReqs = generateGetRequests(isKillSwitchActive());

	concatenated.insert(concatenated.end(), getterReqs.begin(), getterReqs.end());

	if (killSwitchRequest != diff.end()
			and killSwitchRequest->second->getPlainData()[1] == USBCommands::bridge::ACTIVE) {
		logger.notice("enabling kill switch");
		killSwitchRequest->second->appendTo(concatenated);
	}

	concatenated.push_back(USBCommands::MESSAGE_END);

	std::ostringstream oss;
	std::copy(concatenated.begin(), concatenated.end() - 1, std::ostream_iterator<int>(oss, ","));
	oss << concatenated.back();

	logger.debug("sending request to the device (" + to_string(concatenated.size()) + " bytes):" + oss.str());

	usbComm->sendData(concatenated);

	auto response = usbComm->receiveData();

	oss.clear();
	std::copy(response.begin(), response.end() - 1, std::ostream_iterator<int>(oss, ","));
	oss << response.back();

	logger.debug("got response from device (" + to_string(response.size()) + " bytes): " + oss.str());

	updateDataStructures(getterRequests, response);
}

RequestMap InterfaceManager::generateDifferentialRequests() {
	RequestMap diff;

	for (auto& newRequest : requests) {
		if (previousRequests.find(newRequest.first) == previousRequests.end()
				or memcmp(previousRequests[newRequest.first]->getPlainData(), newRequest.second->getPlainData(),
						newRequest.second->getSize())) {
			diff[newRequest.first] = newRequest.second;
		}
	}

	previousRequests = requests;

	return diff;
}

} /* namespace bridge */
