#include <iostream>
#include <functional>
#include <chrono>
#include <boost/format.hpp>

#include "USBCommunicator.hpp"
#include "usb-settings.hpp"
#include "utils.hpp"

using namespace std;
using namespace bridge;
using boost::format;

const int BUFFER_SIZE = USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE;
const int MESSAGE_TIMEOUT = 2000; // ms

WALLAROO_REGISTER(USBCommunicator);

bridge::USBCommunicator::USBCommunicator()
		: logger(log4cpp::Category::getInstance("USBCommunicator")) {
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
				libusb_get_string_descriptor_ascii(devHandle, desc.iManufacturer, (unsigned char *) vendorString,
						sizeof(vendorString));

				libusb_get_string_descriptor_ascii(devHandle, desc.iProduct, (unsigned char *) productString,
						sizeof(productString));
			}

			if ((string(USB_SETTINGS_VENDOR_NAME).compare(vendorString) == 0)
					and (string(USB_SETTINGS_DEVICE_NAME).compare(productString) == 0)) {
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
	} else {
		logger.notice(
				(format("Found SZARK device: 0x%04X/0x%04X - %s [%s].") % USB_SETTINGS_VENDOR_ID
						% USB_SETTINGS_DEVICE_ID % USB_SETTINGS_DEVICE_NAME % USB_SETTINGS_VENDOR_NAME).str());
	}

	int status = 0;

	status = libusb_detach_kernel_driver(devHandle, 0);
	if (status < 0 and status != LIBUSB_ERROR_NOT_FOUND) {
		libusb_close(devHandle);
		throw CommException((format("error at detaching kernel driver (%s)") % libusb_error_name(status)).str());
	} else {
		logger.notice("Device successfully detached from kernel driver.");
	}

	status = libusb_set_configuration(devHandle, 1);
	if (status < 0) {
		libusb_close(devHandle);
		throw CommException((format("error at selecting configuration (%s)") % libusb_error_name(status)).str());
	} else {
		logger.notice("Configuration selected.");
	}

	status = libusb_claim_interface(devHandle, 0);
	if (status < 0) {
		libusb_close(devHandle);
		throw CommException((format("error at claiming interface (%s)") % libusb_error_name(status)).str());
	} else {
		logger.notice("Interface claimed. Device ready to go. Instance created.");
	}
}

bridge::USBCommunicator::~USBCommunicator() {
	libusb_release_interface(devHandle, 0);
	libusb_close(devHandle);
	libusb_exit(nullptr);

	logger.notice("Device released. Instance destroyed.");
}

void bridge::USBCommunicator::sendData(vector<uint8_t> &data) {
	data.resize(BUFFER_SIZE, 0);

	int transferred, status;

	auto microseconds = common::utils::measureTime<chrono::microseconds>([&]() {
		status = libusb_bulk_transfer(devHandle, (USB_SETTINGS_HOST_TO_DEVICE_ENDPOINT_NO | LIBUSB_ENDPOINT_OUT),
				&data[0], USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE, &transferred, MESSAGE_TIMEOUT);
	});

	logger.info(string("Sending data in ") + to_string(microseconds) + " us.");
	logger.debug(string("Sending data: ") + common::utils::toString<uint8_t>(data));

	if (transferred != USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE) {
		throw CommException(
				(format("sent %d bytes to the device instead of %d") % transferred
						% USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE).str());
	}

	if (status < 0) {
		throw CommException((format("error at sending data to the device (%s)") % libusb_error_name(status)).str());
	}
}

vector<uint8_t> bridge::USBCommunicator::receiveData() {
	uint8_t data[BUFFER_SIZE];

	int transferred, status;

	auto microseconds = common::utils::measureTime<chrono::microseconds>([&]() {
		status = libusb_bulk_transfer(devHandle, (USB_SETTINGS_DEVICE_TO_HOST_ENDPOINT_NO | LIBUSB_ENDPOINT_IN),
				data,
				USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE, &transferred, MESSAGE_TIMEOUT);
	});

	logger.info(string("Received data in ") + to_string(microseconds) + " us.");

	if (transferred != USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE) {
		throw CommException(
				(format("received %d bytes from the device instead of %d") % transferred
						% USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE).str());
	}

	if (status < 0) {
		throw CommException((format("error at receiving data from the device (%s)") % libusb_error_name(status)).str());
	}

	vector<uint8_t> vec(BUFFER_SIZE);

	vec.assign(data, data + transferred);

	logger.debug(string("Received data: ") + common::utils::toString<uint8_t>(vec));

	return vec;
}

