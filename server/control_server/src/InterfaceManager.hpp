#pragma once

#include "Interface.hpp"
#include "USBCommunicator.hpp"
#include "Configuration.hpp"
#include "SharedInterfaceProvider.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <wallaroo/registered.h>
#include <log4cpp/Category.hh>

#include <functional>

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

        wallaroo::Plug<common::bridge::InterfaceProvider> interfaceProvider;

        common::bridge::Interface *interface;

        std::pair<std::vector<uint8_t>, std::vector<USBCommands::Request>> generateGetRequests(bool killSwitchActive);

        common::bridge::RequestMap previousRequests;

        virtual void Init();

        common::bridge::RequestMap generateDifferentialRequests(bool killSwitchActive);
    };

} /* namespace bridge */
