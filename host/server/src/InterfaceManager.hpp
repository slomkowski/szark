/*
 * InterfaceManager.hpp
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

#ifndef INTERFACEMANAGER_HPP_
#define INTERFACEMANAGER_HPP_

#include "Interface.hpp"

namespace bridge {

	class InterfaceManager: public bridge::Interface {
	public:
		InterfaceManager();
		virtual ~InterfaceManager();

		void stageChanges();
	private:
		std::vector<uint8_t> generateGetRequests(bool killSwitchActive);
		std::vector<USBCommands::Request> getterRequests;
		long counter = 0;
	};

} /* namespace bridge */
#endif /* INTERFACEMANAGER_HPP_ */
