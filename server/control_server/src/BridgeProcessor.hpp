#pragma once

#include "IRequestProcessor.hpp"
#include "USBCommunicator.hpp"
#include "InterfaceManager.hpp"

#include <log4cpp/Category.hh>
#include <wallaroo/device.h>
#include <json/value.h>

#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

namespace bridge {
    using namespace common::bridge;

    class BridgeProcessor : public processing::IRequestProcessor, public wallaroo::Device {
    public:
        BridgeProcessor();

        ~BridgeProcessor();

        virtual void process(Json::Value &request, boost::asio::ip::address address,
                             Json::Value &response) override;

    private:
        log4cpp::Category &logger;

        wallaroo::Plug<ICommunicator> usbComm;
        wallaroo::Plug<IInterfaceManager> interfaceManager;

        std::unique_ptr<std::thread> maintenanceThread;
        std::mutex maintenanceMutex;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastProcessFunctionExecution;

        volatile bool finishCycleThread = false;
        volatile bool firstMaintenanceTask = true;

        void Init() override;

        void maintenanceThreadFunction();

        Interface &iface() {
            return interfaceManager->iface();
        }

        void createReport(Json::Value &r);

        void parseRequest(Json::Value &r);

        template<typename T>
        void tryAssign(const Json::Value &key, std::function<void(T)> setter);

        void tryAssignDirection(const Json::Value &key, std::function<void(Direction)> setter);

        void fillAllDevices(
                std::function<void(std::string name, Joint j)> fillArm,
                std::function<void(std::string name, Motor m)> fillMotor,
                std::function<void(std::string name, ExpanderDevice d)> fillExpander);
    };

} /* namespace bridge */
