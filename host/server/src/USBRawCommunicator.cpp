/*
 * RawCommunicator.cpp
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#include "USBRawCommunicator.hpp"

namespace USB {

	RawCommunicator::RawCommunicator() {
		devHandle = nullptr;

		bool deviceFound = false;

		libusb_init(nullptr);

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
	}

	RawCommunicator::~RawCommunicator() {
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
		int status = libusb_control_transfer(devHandle,
			LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, request, 0, 0,
			reinterpret_cast<unsigned char*>(data), length, MESSAGE_TIMEOUT);

		if (status < 0) {
			std::string mesg = "error at sending data to the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}
	}

	unsigned int RawCommunicator::recvMessage(USBCommands::USBRequest request, uint8_t *data, unsigned int maxLength) {

		int status = libusb_control_transfer(devHandle,
			LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, request, 0, 0, data,
			maxLength, MESSAGE_TIMEOUT);

		if (status < 0) {
			std::string mesg = "error at receiving data from the device (";
			mesg += libusb_error_name(status);
			mesg += ")";
			throw CommException(mesg);
		}

		return status;
	}

} /* namespace USB */
