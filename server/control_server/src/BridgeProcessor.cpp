#include "BridgeProcessor.hpp"
#include "Interface.hpp"
#include "convert.hpp"

#include <boost/format.hpp>
#include <json/value.h>

#include <pthread.h>

#include <chrono>
#include <functional>

using namespace std;
using namespace bridge;
using namespace common::bridge;

using std::chrono::high_resolution_clock;

/**
* If the process() function in the given timeout, the maintainance thread starts operation.
* It prevents the device from going to stopped state, queries for battery etc.
*/
constexpr chrono::milliseconds TIMEOUT(25);
constexpr chrono::milliseconds MAINTENANCE_TASK_INTERVAL(25);
constexpr bool MAINTENANCE_TASK_ENABLED = true;

WALLAROO_REGISTER(BridgeProcessor);

bridge::BridgeProcessor::BridgeProcessor()
        : logger(log4cpp::Category::getInstance("BridgeProcessor")),
          usbComm("communicator", RegistrationToken()),
          interfaceManager("interfaceManager", RegistrationToken()),
          lastProcessFunctionExecution(high_resolution_clock::now()) {
}

void bridge::BridgeProcessor::Init() {
    if (MAINTENANCE_TASK_ENABLED) {
        maintenanceThread.reset(new thread(&BridgeProcessor::maintenanceThreadFunction, this));
        int result = pthread_setname_np(maintenanceThread->native_handle(), "bMaintenance");
        if (result != 0) {
            logger.error("Cannot set thread name: %s.", strerror(result));
        }
    }

    logger.notice("Instance created.");
}

bridge::BridgeProcessor::~BridgeProcessor() {
    finishCycleThread = true;

    logger.notice("Waiting for maintenance task to stop.");

    if (maintenanceThread.get() != nullptr) {
        maintenanceThread->join();
    }

    logger.notice("Instance destroyed.");
}

void bridge::BridgeProcessor::process(Json::Value &request, boost::asio::ip::address address,
                                      Json::Value &response) {
    SharedScopedMutex lk(iface().mutex);

    firstMaintenanceTask = true;

    logger.info("Processing request.");

    parseRequest(request);

    interfaceManager->syncWithDevice([&](vector<uint8_t> r) {
        usbComm->sendData(r);
        return usbComm->receiveData();
    });

    createReport(response);

    lastProcessFunctionExecution = high_resolution_clock::now();
}

void bridge::BridgeProcessor::maintenanceThreadFunction() {
    while (true) {
        this_thread::sleep_for(MAINTENANCE_TASK_INTERVAL);

        if (finishCycleThread) {
            break;
        }

        if ((not usbComm.WiringOk()) or ((high_resolution_clock::now() - lastProcessFunctionExecution) < TIMEOUT)) {
            this_thread::yield();
            continue;
        }

        SharedScopedMutex lk(iface().mutex);

        if (firstMaintenanceTask) {
            logger.notice("No requests, starting performing maintenance task.");
            firstMaintenanceTask = false;
        }

        logger.info("Performing maintenance task.");
        // TODO dodać - jeżeli przez 2s nie ma sygnału, to zatrzymaj wszystko

        interfaceManager->syncWithDevice([&](vector<uint8_t> r) {
            usbComm->sendData(r);
            return usbComm->receiveData();
        });
    }
}

using bridge::ExpanderDevice;
using bridge::Motor;
using bridge::Joint;
using bridge::Button;

template<typename T>
struct jsonType {
    static const Json::ValueType value = Json::stringValue;

    static void execute(const Json::Value &key, function<void(T)> &setter) {
        setter(key.asString());
    }
};

template<>
struct jsonType<bool> {
    static const Json::ValueType value = Json::booleanValue;

    static void execute(const Json::Value &key, function<void(bool)> &setter) {
        setter(key.asBool());
    }
};

template<>
struct jsonType<int> {
    static const Json::ValueType value = Json::intValue;

    static void execute(const Json::Value &key, function<void(int)> &setter) {
        setter(key.asInt());
    }
};

template<typename T>
void bridge::BridgeProcessor::tryAssign(const Json::Value &key, function<void(T)> setter) {
    if (key.empty()) {
        return;
    }

    if (not key.isConvertibleTo(jsonType<T>::value)) {
        logger.error("Value for " + key.toStyledString() + " is in invalid format.");
        return;
    }

    jsonType<T>::execute(key, setter);
}

void bridge::BridgeProcessor::tryAssignDirection(const Json::Value &key, function<void(Direction)> setter) {
    if (key.empty()) {
        return;
    }

    try {
        auto dir = stringToDirection(key.asString());
        setter(dir);
    } catch (runtime_error &e) {
        logger.error("Value for " + key.toStyledString() + ": " + e.what() + ".");
    }
}

void bridge::BridgeProcessor::parseRequest(Json::Value &r) {
    using namespace std::placeholders;
    // TODO wkładanie requestów do interfejsu

    tryAssign<bool>(r["ks_en"], std::bind(&Interface::setKillSwitch, &iface(), _1));
    tryAssign<string>(r["lcd"], std::bind(&Interface::setLCDText, &iface(), _1));

    auto fillArm = [&](string name, Joint j) {
        tryAssign<int>(r["arm"][name]["speed"],
                       bind(&Interface::ArmClass::SingleJoint::setSpeed, &iface().arm[j], _1));

        tryAssignDirection(r["arm"][name]["dir"],
                           bind(&Interface::ArmClass::SingleJoint::setDirection, &iface().arm[j], _1));

        if (r["arm"][name]["dir"].empty()) {
            tryAssign<int>(r["arm"][name]["pos"],
                           bind(&Interface::ArmClass::SingleJoint::setPosition, &iface().arm[j], _1));
        }
    };

    // TODO arm ogólne ustawienia kalibracja itd.

    auto fillMotor = [&](string name, Motor m) {
        tryAssign<int>(r["motor"][name]["speed"],
                       bind(&Interface::MotorClass::SingleMotor::setSpeed, &iface().motor[m], _1));

        tryAssignDirection(r["motor"][name]["dir"],
                           bind(&Interface::MotorClass::SingleMotor::setDirection, &iface().motor[m], _1));
    };

    auto fillExpander = [&](string name, ExpanderDevice d) {
        tryAssign<bool>(r["light"][name],
                        bind(&Interface::ExpanderClass::Device::setEnabled, &iface().expander[d], _1));
    };

    tryAssign<bool>(r["arm"]["b_cal"], [&](bool startCalibration) {
        if (startCalibration) {
            iface().arm.calibrate();
        }
    });

    fillAllDevices(fillArm, fillMotor, fillExpander);
}

void bridge::BridgeProcessor::createReport(Json::Value &r) {

    auto fillExpander = [&](string name, ExpanderDevice d) {
        r["light"][name] = iface().expander[d].isEnabled();
    };

    auto fillButtons = [&](string name, Button d) {
        if (iface().isButtonPressed(d)) {
            r["button"].append(name);
        }
    };

    auto fillMotor = [&](string name, Motor m) {
        r["motor"][name]["speed"] = iface().motor[m].getSpeed();
        r["motor"][name]["dir"] = directionToString(iface().motor[m].getDirection());
    };

    auto fillArm = [&](string name, Joint j) {
        r["arm"][name]["speed"] = iface().arm[j].getSpeed();
        r["arm"][name]["pos"] = iface().arm[j].getPosition();
        r["arm"][name]["dir"] = directionToString(iface().arm[j].getDirection());
    };

    r["arm"]["cal_st"] = armCalibrationStatusToString(iface().arm.getCalibrationStatus());
    r["arm"]["mode"] = armDriverModeToString(iface().arm.getMode());

    fillAllDevices(fillArm, fillMotor, fillExpander);

    fillButtons("up", Button::UP);
    fillButtons("down", Button::DOWN);
    fillButtons("enter", Button::ENTER);

    r["batt"]["volt"] = iface().getVoltage();
    r["batt"]["curr"] = iface().getCurrent();

    if (iface().isKillSwitchActive()) {
        if (iface().isKillSwitchCausedByHardware()) {
            r["ks_stat"] = "hardware";
        } else {
            r["ks_stat"] = "software";
        }
    } else {
        r["ks_stat"] = "inactive";
    }
}

void bridge::BridgeProcessor::fillAllDevices(
        std::function<void(string, Joint)> fillArm,
        std::function<void(string, Motor)> fillMotor,
        std::function<void(string, ExpanderDevice)> fillExpander) {

    fillExpander("right", ExpanderDevice::LIGHT_RIGHT);
    fillExpander("left", ExpanderDevice::LIGHT_LEFT);
    fillExpander("camera", ExpanderDevice::LIGHT_CAMERA);

    fillMotor("left", Motor::LEFT);
    fillMotor("right", Motor::RIGHT);

    fillArm("shoulder", Joint::SHOULDER);
    fillArm("elbow", Joint::ELBOW);
    fillArm("gripper", Joint::GRIPPER);
}
