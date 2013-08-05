/*
 * USB Geiger counter
 * 2013 Michał Słomkowski
 * This code is distributed under the terms of GNU General Public License version 3.0.
 */

#ifndef USBCOMMUNICATOR_H_
#define USBCOMMUNICATOR_H_

#include <libusb.h>
#include <stdexcept>
#include <string>

namespace USB {

	const int VENDOR_ID = 0x16c0;
	const int DEVICE_ID = 0x05df;

	const std::string VENDOR_NAME = "slomkowski.eu";
	const std::string DEVICE_NAME = "SZARK Bridge";

	typedef enum {
		SOME_TYPE = 1
	} Request;

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

	class RawCommunicator {
	public:
		/**
		 * Initiates the class and opens the device. Warning! The constructor assumes that only one bridge device
		 * is connected to the bus. Otherwise, it opens the first-found one.
		 */
		RawCommunicator() throw (CommException);

		virtual ~RawCommunicator();

		void sendMessage(Request request, unsigned int value);
		unsigned int recvMessage(Request request);

	private:
		libusb_device_handle *devHandle;
	};

}

#endif /* USBCOMMUNICATOR_H_ */
