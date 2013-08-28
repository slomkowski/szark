/*
 * RawCommunicator.h
 *
 *  Created on: 12-08-2013
 *      Author: michal
 */

#ifndef RAWCOMMUNICATOR_H_
#define RAWCOMMUNICATOR_H_

extern "C" {
#include <libusb.h>
}
#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>
#include <boost/noncopyable.hpp>

#include "usb-commands.h"

namespace USB {

	const int VENDOR_ID = 0x16c0;
	const int DEVICE_ID = 0x05df;

	const std::string VENDOR_NAME = "slomkowski.eu";
	const std::string DEVICE_NAME = "SZARK Bridge";

	const int BUFFER_SIZE = 128;
	const int MESSAGE_TIMEOUT = 2000; // ms

	/**
	 * This exception is thrown if an USB communication error occurs. You can view the error description
	 * by calling exception.what()
	 */
	class CommException: public std::runtime_error {
	public:
		CommException(const std::string& message) :
			std::runtime_error(message) {
		}
	};

	class RawCommunicator: boost::noncopyable {
	public:
		RawCommunicator();
		virtual ~RawCommunicator();

		void sendMessage(USBCommands::USBRequest request, unsigned int value);

		void sendMessage(USBCommands::USBRequest request, uint8_t *data, unsigned int length);

		unsigned int recvMessage(USBCommands::USBRequest request, uint8_t *data, unsigned int maxLength);

	private:
		libusb_device_handle *devHandle;
	};

	class Communicator: RawCommunicator {
	public:
		void sendData(const std::vector<uint8_t>& data);

		std::vector<uint8_t> receiveData();

		bool isResponseReady();
	};

} /* namespace USB */
#endif /* RAWCOMMUNICATOR_H_ */
