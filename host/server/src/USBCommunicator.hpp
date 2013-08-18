/*
 * USBCommunicator.hpp
 *
 *  Created on: 18-08-2013
 *      Author: michal
 */

#ifndef USBCOMMUNICATOR_HPP_
#define USBCOMMUNICATOR_HPP_

namespace USB {

	class USBCommunicator {
	public:
		USBCommunicator();
		virtual ~USBCommunicator();

		void sendRequest();

		void receiveRequest();
	};

} /* namespace USB */
#endif /* USBCOMMUNICATOR_HPP_ */
