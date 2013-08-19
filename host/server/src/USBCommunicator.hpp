/*
 * USBCommunicator.hpp
 *
 *  Created on: 18-08-2013
 *      Author: michal
 */

#ifndef USBCOMMUNICATOR_HPP_
#define USBCOMMUNICATOR_HPP_

namespace USB {

	class Communicator {
	public:
		Communicator();
		virtual ~Communicator();

		void sendRequest();

		void receiveRequest();
	};

	enum class Direction {
		STOP, FORWARD, BACKWARD
	};

	class Motor {
	private:
		Direction direction;
		unsigned int speed;
		unsigned int position;
		bool setPosition;
	public:
		int fase;
	};

} /* namespace USB */
#endif /* USBCOMMUNICATOR_HPP_ */
