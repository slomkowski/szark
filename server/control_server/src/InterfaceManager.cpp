#include "InterfaceManager.hpp"
#include "utils.hpp"

#include <thread>
#include <queue>

using namespace std;
using namespace boost;
using namespace boost::interprocess;
using namespace bridge;
using namespace common::bridge;

WALLAROO_REGISTER(InterfaceManager);

bridge::InterfaceManager::InterfaceManager()
        : logger(log4cpp::Category::getInstance("InterfaceManager")),
          config("config", RegistrationToken()),
          interfaceProvider("interfaceProvider", RegistrationToken()) {
}

void InterfaceManager::Init() {
    interface = interfaceProvider->getInterface();
    logger.notice("Instance created.");
}

bridge::InterfaceManager::~InterfaceManager() {
    logger.notice("Instance destroyed.");
}

pair<vector<uint8_t>, vector<USBCommands::Request>> bridge::InterfaceManager::generateGetRequests(
        bool killSwitchActive) {
    static long counter = 0;

    vector<uint8_t> commands;
    std::vector<USBCommands::Request> responseOrder;

    commands.push_back(USBCommands::Request::BRIDGE_GET_STATE);
    responseOrder.push_back(USBCommands::Request::BRIDGE_GET_STATE);

    // if the kill switch is active, devices are in reset state and won't respond anyway
    if (killSwitchActive) {
        return make_pair(commands, responseOrder);
    }

    commands.push_back(USBCommands::Request::MOTOR_DRIVER_GET);
    responseOrder.push_back(USBCommands::Request::MOTOR_DRIVER_GET);
    if (counter % 2) {
        commands.push_back(motor::MOTOR1);
    } else {
        commands.push_back(motor::MOTOR2);
    }

    commands.push_back(USBCommands::Request::ARM_DRIVER_GET);
    responseOrder.push_back(USBCommands::Request::ARM_DRIVER_GET);

    switch (counter % 4) {
        case 0:
            commands.push_back(arm::ELBOW);
            break;
        case 1:
            commands.push_back(arm::GRIPPER);
            break;
        case 2:
            commands.push_back(arm::SHOULDER);
            break;
        case 3:
            commands.pop_back();
            commands.push_back(USBCommands::Request::ARM_DRIVER_GET_GENERAL_STATE);
            responseOrder.pop_back();
            responseOrder.push_back(USBCommands::Request::ARM_DRIVER_GET_GENERAL_STATE);
            break;
    };

    counter++;

    return make_pair(commands, responseOrder);
}

void bridge::InterfaceManager::syncWithDevice(BridgeSyncFunction syncFunction) {

    vector<uint8_t> concatenated;
    std::priority_queue<DataHolder, vector<DataHolder>, DataHolderComparer> sortedRequests;

    auto diff = generateDifferentialRequests(interface->isKillSwitchActive());

    for (auto &r : diff) {
        sortedRequests.push(r.second);
    }

    while (not sortedRequests.empty()) {
        sortedRequests.top().appendTo(concatenated);
        sortedRequests.pop();
    }

    auto getterReqs = generateGetRequests(interface->isKillSwitchActive());
    concatenated.insert(concatenated.end(), getterReqs.first.begin(), getterReqs.first.end());

    concatenated.push_back(USBCommands::MESSAGE_END);

    logger.debug("Sending request to the device (%d bytes): %s.",
                 concatenated.size(), common::utils::toString<uint8_t>(concatenated).c_str());

    auto response = syncFunction(concatenated);

    logger.debug("Got response from device (%d bytes): %s.",
                 response.size(), common::utils::toString<uint8_t>(response).c_str());

    interface->updateDataStructures(getterReqs.second, response);
}

RequestMap bridge::InterfaceManager::generateDifferentialRequests(bool killSwitchActive) {
    RequestMap diff;

    for (auto &newRequest : *(interface->getRequestMap())) {
        if (previousRequests.find(newRequest.first) == previousRequests.end()) {
            logger.debug("Key '%s' not in the previous state. Adding.", newRequest.first.c_str());
            diff[newRequest.first] = newRequest.second;
        }
        else {
            if (previousRequests[newRequest.first].equals(newRequest.second)) {
                logger.debug("Key '%s' identical to the previous one. Skipping.", newRequest.first.c_str());
            } else {
                logger.debug("Key '%s' differs from previous state's one. Adding.", newRequest.first.c_str());
                diff[newRequest.first] = newRequest.second;
            }
        }
    }

    previousRequests.clear();

    for (auto req : *(interface->getRequestMap())) {
        previousRequests.insert(req);
    }

    if (killSwitchActive) {
        for (auto &r : *(interface->getRequestMap())) {
            if (r.second.isKillSwitchDependent()) {
                logger.debug("Removing key '%s' because kill switch is active.", r.first.c_str());
                diff.erase(r.first);
                previousRequests.erase(r.first);
            }
        }
    }
    return diff;
}

Interface &InterfaceManager::iface() {
    return *interface;
}

