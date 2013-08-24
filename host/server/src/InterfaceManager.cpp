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
#include "USBRawCommunicator.hpp"

using namespace std;

namespace bridge {

	InterfaceManager::InterfaceManager() {
		// TODO Auto-generated constructor stub

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

		// ensure that disabling kill switch is the first command
		/*auto killSwitchRequest = impl->requests.find(KILLSWITCH_STRING);
		if (killSwitchRequest != impl->requests.end()
			and killSwitchRequest->second->getPlainData()[1] == USBCommands::bridge::INACTIVE) {
			killSwitchRequest->second->appendTo(concatenated);
			impl->killSwitchActive = false;
		}

		for (auto& request : impl->requests) {
			if (request.first == KILLSWITCH_STRING) {
				continue;
			}

			cout << request.first << ": " << request.second->getSize() << endl;
			request.second->appendTo(concatenated);
		}

		// append getter requests
		vector<uint8_t> getterReqs = impl->generateGetRequests(impl->killSwitchActive);
		concatenated.insert(concatenated.end(), getterReqs.begin(), getterReqs.end());

		if (killSwitchRequest != impl->requests.end()
			and killSwitchRequest->second->getPlainData()[1] == USBCommands::bridge::ACTIVE) {
			killSwitchRequest->second->appendTo(concatenated);
			impl->killSwitchActive = true;
		}

		cout << concatenated.size() << " " << concatenated.capacity() << endl;
		for (unsigned int i = 0; i < concatenated.size(); i++) {
			cout << i << "i: " << (int) concatenated[i] << endl;
		}*/

		USB::RawCommunicator comm;
		comm.sendMessage(USBCommands::USB_WRITE, (uint8_t*) &concatenated[0], concatenated.size());
	}

} /* namespace bridge */
