#ifndef INTERFACEMANAGER_HPP_
#define INTERFACEMANAGER_HPP_

#include <functional>

#include <log4cpp/Category.hh>

#include "Interface.hpp"
#include "USBCommunicator.hpp"

namespace bridge {

	class InterfaceManager : public bridge::Interface {
	public:
		InterfaceManager();

		virtual ~InterfaceManager();

		void syncWithDevice(std::function<std::vector<uint8_t>(std::vector<uint8_t>)> syncFunction);

	private:
		log4cpp::Category &logger;

		std::pair<std::vector<uint8_t>, std::vector<USBCommands::Request>> generateGetRequests(bool killSwitchActive);

		RequestMap previousRequests;

		RequestMap generateDifferentialRequests(bool killSwitchActive);
	};

} /* namespace bridge */
#endif /* INTERFACEMANAGER_HPP_ */
