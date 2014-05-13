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
#include "DataHolder.hpp"

#include <cstring>
#include <thread>
#include <chrono>
#include <tuple>
#include <memory>
#include <queue>

#include <boost/format.hpp>

#include "utils.hpp"

using namespace std;
using namespace boost;

namespace bridge {

InterfaceManager::InterfaceManager()
		: logger(log4cpp::Category::getInstance("InterfaceManager")) {
	logger.notice("Instance created.");
}

InterfaceManager::~InterfaceManager() {
	logger.notice("Instance destroyed.");
}

pair<vector<uint8_t>, vector<USBCommands::Request>> InterfaceManager::generateGetRequests(bool killSwitchActive) {
	static long counter = 0;

	vector<uint8_t> commands;
	std::vector<USBCommands::Request> responseOrder;

	commands.push_back(USBCommands::Request::BRIDGE_GET_STATE);
	responseOrder.push_back(USBCommands::Request::BRIDGE_GET_STATE);

	// if the kill switch is active, devices are in reset state and won't repond anyway
	if (killSwitchActive) {
		return make_pair(commands, responseOrder);
	}

	commands.push_back(USBCommands::Request::MOTOR_DRIVER_GET);
	responseOrder.push_back(USBCommands::Request::MOTOR_DRIVER_GET);
	if (counter % 2) {
		commands.push_back(motor::MOTOR1);
	} else {
		commands.push_back(motor::MOTOR2);
	}

	commands.push_back(USBCommands::Request::ARM_DRIVER_GET);
	responseOrder.push_back(USBCommands::Request::ARM_DRIVER_GET);

	switch (counter % 5) {
	case 0:
		commands.push_back(arm::ELBOW);
		break;
	case 1:
		commands.push_back(arm::WRIST);
		break;
	case 2:
		commands.push_back(arm::GRIPPER);
		break;
	case 3:
		commands.push_back(arm::SHOULDER);
		break;
	case 4:
		commands.pop_back();
		commands.push_back(USBCommands::Request::ARM_DRIVER_GET_GENERAL_STATE);
		responseOrder.pop_back();
		responseOrder.push_back(USBCommands::Request::ARM_DRIVER_GET_GENERAL_STATE);
		break;
	};

	counter++;

	return make_pair(commands, responseOrder);
}

void InterfaceManager::syncWithDevice(std::function<std::vector<uint8_t>(std::vector<uint8_t>)> syncFunction) {

	vector<uint8_t> concatenated;
	std::priority_queue<std::shared_ptr<DataHolder>, vector<std::shared_ptr<DataHolder>>, DataHolderComparer> sortedRequests;

	auto diff = generateDifferentialRequests(isKillSwitchActive());

	for (auto& r : diff) {
		sortedRequests.push(r.second);
	}

	while (not sortedRequests.empty()) {
		sortedRequests.top()->appendTo(concatenated);
		sortedRequests.pop();
	}

	auto getterReqs = generateGetRequests(isKillSwitchActive());
	concatenated.insert(concatenated.end(), getterReqs.first.begin(), getterReqs.first.end());

	concatenated.push_back(USBCommands::MESSAGE_END);

	logger.debug(
			(format("Sending request to the device (%d bytes): %s.") % concatenated.size()
					% utils::toString<uint8_t>(concatenated)).str());

	auto response = syncFunction(concatenated);

	logger.debug(
			(format("Got response from device (%d bytes): %s.") % response.size() % utils::toString<uint8_t>(response)).str());

	updateDataStructures(getterReqs.second, response);
}

RequestMap InterfaceManager::generateDifferentialRequests(bool killSwitchActive) {
	RequestMap diff;

	for (auto& newRequest : requests) {
		if (previousRequests.find(newRequest.first) == previousRequests.end()) {
			logger.debug((format("Key '%s' not in the previous state. Adding.") % newRequest.first).str());
			diff[newRequest.first] = newRequest.second;
		}
		else {
			if (previousRequests[newRequest.first]->equals(*newRequest.second)) {
				logger.debug((format("Key '%s' identical to the previous one. Skipping.") % newRequest.first).str());
			} else {
				logger.debug((format("Key '%s' differs from previous state's one. Adding.") % newRequest.first).str());
				diff[newRequest.first] = newRequest.second;
			}
		}
	}

	previousRequests = requests;
	if (killSwitchActive) {
		for (auto& r : requests) {
			if (r.second->isKillSwitchDependent()) {
				logger.debug((format("Removing key '%s' because kill switch is active.") % r.first).str());
				diff.erase(r.first);
				previousRequests.erase(r.first);
			}
		}
	}
	return diff;
}

} /* namespace bridge */
