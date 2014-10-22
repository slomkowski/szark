#ifndef INTERFACEMANAGER_HPP_
#define INTERFACEMANAGER_HPP_

#include <functional>

#include <wallaroo/registered.h>
#include <log4cpp/Category.hh>

#include "Interface.hpp"
#include "USBCommunicator.hpp"
#include "Configuration.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>

namespace bridge {

	typedef std::function<std::vector<uint8_t>(std::vector<uint8_t>)> BridgeSyncFunction;

	class IInterfaceManager : boost::noncopyable {
	public:
		virtual ~IInterfaceManager() = default;

		virtual void syncWithDevice(BridgeSyncFunction syncFunction) = 0;

		virtual common::bridge::Interface &iface() = 0;
	};

	class InterfaceManager : public IInterfaceManager, public wallaroo::Device {
	public:
		InterfaceManager();

		~InterfaceManager();

		void syncWithDevice(BridgeSyncFunction syncFunction);

		common::bridge::Interface &iface();

	private:
		log4cpp::Category &logger;

		wallaroo::Plug<common::config::Configuration> config;

		boost::interprocess::managed_shared_memory *memorySegment;

		common::bridge::Interface *interface;

		std::pair<std::vector<uint8_t>, std::vector<USBCommands::Request>> generateGetRequests(bool killSwitchActive);

		common::bridge::RequestMap previousRequests;

		common::bridge::RequestMap generateDifferentialRequests(bool killSwitchActive);
	};

} /* namespace bridge */
#endif /* INTERFACEMANAGER_HPP_ */
