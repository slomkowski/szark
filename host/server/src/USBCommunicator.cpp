/*
 * USBCommunicator.cpp
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

#include <iostream>

#include "USBCommunicator.hpp"
#include "usb-settings.hpp"

namespace USB {

	const int BUFFER_SIZE = USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE;
	const int MESSAGE_TIMEOUT = 2000; // ms

	Communicator::Communicator() {
		devHandle = nullptr;

		bool deviceFound = false;

		libusb_init(nullptr);

		libusb_set_debug(nullptr, 3);

		libusb_device **listOfDevices;
		auto noOfDevices = libusb_get_device_list(nullptr, &listOfDevices);

		for (auto i = 0; i < noOfDevices; i++) {
			struct libusb_device_descriptor desc;

			int status = libusb_get_device_descriptor(listOfDevices[i], &desc);
			if (status < 0) {
				continue;
			}

			if ((desc.idVendor == USB_SETTINGS_VENDOR_ID) and (desc.idProduct == USB_SETTINGS_DEVICE_ID)) {
				status = libusb_open(listOfDevices[i], &devHandle);
				if (status != 0) {
					continue;
				}

				char vendorString[BUFFER_SIZE] = "", productString[BUFFER_SIZE] = "";

				if ((desc.iManufacturer > 0) && (desc.iProduct > 0)) {
					libusb_get_string_descriptor_ascii(devHandle, desc.iManufacturer, (unsigned char*) vendorString,
						sizeof(vendorString));

					libusb_get_string_descriptor_ascii(devHandle, desc.iProduct, (unsigned char*) productString,
						sizeof(productString));
				}

				if ((std::string(USB_SETTINGS_VENDOR_NAME).compare(vendorString) == 0)
					and (std::string(USB_SETTINGS_DEVICE_NAME).compare(productString) == 0)) {
					deviceFound = true;
					break;
				} else {
					libusb_close(devHandle);
				}
			}
		}

		libusb_free_device_list(listOfDevices, 1);

		if (deviceFound == false) {
			throw CommException("SZARK bridge device not found");
		}

		int status = 0;

		status = libusb_detach_kernel_driver(devHandle, 0);
		if (status < 0 and status != LIBUSB_ERROR_NOT_FOUND) {
			libusb_close(devHandle);

			std::string mesg = "error at detaching kernel driver (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}

		status = libusb_set_configuration(devHandle, 1);
		if (status < 0) {
			libusb_close(devHandle);

			std::string mesg = "error at selecting configuration (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}

		status = libusb_claim_interface(devHandle, 0);
		if (status < 0) {
			libusb_close(devHandle);

			std::string mesg = "error at claiming interface (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}
	}

	Communicator::~Communicator() {
		libusb_release_interface(devHandle, 0);
		libusb_close(devHandle);
		libusb_exit(nullptr);
	}

	void Communicator::sendData(std::vector<uint8_t>& data) {
		data.resize(BUFFER_SIZE, 0);

		int transferred;

		int status = libusb_bulk_transfer(devHandle, (USB_SETTINGS_HOST_TO_DEVICE_ENDPOINT_NO | LIBUSB_ENDPOINT_OUT),
			&data[0], USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE, &transferred, MESSAGE_TIMEOUT);

		if (transferred != USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE) {
			std::string mesg = std::string("sent ") + std::to_string(transferred);
			mesg += " bytes to the device instead of " + std::to_string(USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE);
			throw CommException(mesg);
		}

		if (status < 0) {
			std::string mesg = "error at sending data to the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}
	}

	std::vector<uint8_t> Communicator::receiveData() {
		uint8_t data[BUFFER_SIZE];

		int transferred;

		int status = libusb_bulk_transfer(devHandle, (USB_SETTINGS_DEVICE_TO_HOST_ENDPOINT_NO | LIBUSB_ENDPOINT_IN),
			data, USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE, &transferred, MESSAGE_TIMEOUT);

		if (transferred != USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE) {
			std::string mesg = std::string("received ") + std::to_string(transferred);
			mesg += " bytes from the device instead of " + std::to_string( USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE);
			throw CommException(mesg);
		}

		if (status < 0) {
			std::string mesg = "error at receiving data from the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}

		std::vector<uint8_t> vec(BUFFER_SIZE);

		vec.assign(data, data + transferred);

		return vec;
	}
} /* namespace USB */
