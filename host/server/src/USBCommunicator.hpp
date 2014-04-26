/*
 * USBCommunicator.hpp
 *
 *  Project: server
 *  Created on: 24-08-2013
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#ifndef USBCOMMUNICATOR_H_
#define USBCOMMUNICATOR_H_

extern "C" {
#include <libusb.h>
}
#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

#include <boost/noncopyable.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>

#include "usb-commands.hpp"

namespace bridge {
/**
 * This exception is thrown if an USB communication error occurs. You can view the error description
 * by calling exception.what()
 */
class CommException: public std::runtime_error {
public:
	CommException(const std::string& message)
			: std::runtime_error(message) {
	}
};

class ICommunicator {
public:
	virtual ~ICommunicator() = default;

	virtual void sendData(std::vector<uint8_t>& data) = 0;

	virtual std::vector<uint8_t> receiveData() = 0;
};

class USBCommunicator: public ICommunicator, public wallaroo::Device {
public:
	USBCommunicator();
	virtual ~USBCommunicator();

	void sendData(std::vector<uint8_t>& data);

	std::vector<uint8_t> receiveData();

private:
	log4cpp::Category& logger;
	libusb_device_handle *devHandle;
};

} /* namespace USB */
#endif /* USBCOMMUNICATOR_H_ */
