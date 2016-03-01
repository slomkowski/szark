#pragma once

#include "IRequestProcessor.hpp"
#include "USBCommunicator.hpp"
#include "InterfaceManager.hpp"

#include <log4cpp/Category.hh>
#include <wallaroo/part.h>
#include <minijson_writer.hpp>

#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

namespace bridge {
    using namespace common::bridge;

    class BridgeProcessor : public processing::IRequestProcessor, public wallaroo::Part {
    public:
        BridgeProcessor();

        ~BridgeProcessor();

        void process(processing::Request &request, minijson::object_writer &response) override;

    private:
        log4cpp::Category &logger;

        wallaroo::Collaborator<ICommunicator> usbComm;
        wallaroo::Collaborator<IInterfaceManager> interfaceManager;

        std::unique_ptr<std::thread> maintenanceThread;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastProcessFunctionExecution;

        volatile bool finishCycleThread = false;
        volatile bool firstMaintenanceTask = true;

        void Init() override;

        void maintenanceThreadFunction();

        Interface &iface() {
            return interfaceManager->iface();
        }

        void createReport(minijson::object_writer &r);

        void parseRequest(rapidjson::Document &r);

        void tryAssignBool(const rapidjson::Value &key, std::function<void(bool)> setter);

        void tryAssignInt(const rapidjson::Value &key, std::function<void(int)> setter);

        void tryAssignDirection(const rapidjson::Value &key, std::function<void(Direction)> setter);

        void fillAllDevices(
                std::function<void(const char *name, Joint j)> fillArm,
                std::function<void(const char *name, Motor m)> fillMotor,
                std::function<void(const char *name, ExpanderDevice d)> fillExpander);
    };

}
