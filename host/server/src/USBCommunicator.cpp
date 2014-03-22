/*
 * RawCommunicator.cpp
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#include "USBCommunicator.hpp"
#include <iostream>

namespace USB {

	RawCommunicator::RawCommunicator() {
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

			if ((desc.idVendor == VENDOR_ID) && (desc.idProduct == DEVICE_ID)) {
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

				if ((VENDOR_NAME.compare(vendorString) == 0) && (DEVICE_NAME.compare(productString) == 0)) {
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

	RawCommunicator::~RawCommunicator() {
		libusb_release_interface(devHandle, 0);
		libusb_close(devHandle);
		libusb_exit(nullptr);
	}

	void RawCommunicator::sendMessage(USBCommands::USBRequest request, unsigned int value) {
		unsigned char buffer[BUFFER_SIZE];

		if (value > 0xffff) {
			throw CommException("device doesn't support values longer than two bytes");
		}

		int status = libusb_control_transfer(devHandle,
			LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, request, value, 0, buffer, 0,
			MESSAGE_TIMEOUT);

		if (status < 0) {
			std::string mesg = "error at sending data to the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}
	}

	void RawCommunicator::sendMessage(USBCommands::USBRequest request, uint8_t *data, unsigned int length) {

		int transferred;
		int status = libusb_bulk_transfer(devHandle, (1 | LIBUSB_ENDPOINT_OUT), data, length, &transferred,
			MESSAGE_TIMEOUT);

		if (status < 0) {
			std::string mesg = "error at sending data to the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}
	}

	unsigned int RawCommunicator::recvMessage(USBCommands::USBRequest request, uint8_t *data, unsigned int maxLength) {

		int length;
		// TODO te 64 bajty wywalić i zrobić z definicjami
		int status = libusb_bulk_transfer(devHandle, (2 | LIBUSB_ENDPOINT_IN), data, 64, &length, MESSAGE_TIMEOUT);

		if (status < 0) {
			std::string mesg = "error at receiving data from the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}
		//std::cout << "l: " << status << std::endl;

		return length;
	}

	void Communicator::sendData(std::vector<uint8_t>& data) {
		data.resize(BUFFER_SIZE, 0);
		sendMessage(USBCommands::USB_WRITE, (uint8_t*) &data[0], data.size());
	}

	std::vector<uint8_t> Communicator::receiveData() {
		uint8_t data[BUFFER_SIZE];

		unsigned int length = recvMessage(USBCommands::USB_READ, data, BUFFER_SIZE);

		std::vector<uint8_t> vec(BUFFER_SIZE);

		vec.assign(data, data + length);

		return vec;
	}

	bool Communicator::isResponseReady() {
		unsigned char ready = 0;

		recvMessage(USBCommands::IS_RESPONSE_READY, &ready, 1);

		return ready != 0;
	}

} /* namespace USB */
