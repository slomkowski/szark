#define WALLAROO_REMOVE_DEPRECATED

#include "BridgeProcessor.hpp"
#include "Interface.hpp"
#include "convert.hpp"
#include "utils.hpp"

#include <boost/format.hpp>
#include <json/value.h>

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
        common::utils::setThreadName(logger, maintenanceThread.get(), "bMaintenance");
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

void bridge::BridgeProcessor::process(processing::Request &request, minijson::object_writer &response) {
    SharedScopedMutex lk(iface().mutex);

    firstMaintenanceTask = true;

    logger.info("Processing request.");

    parseRequest(request.reqJson);

    interfaceManager->syncWithDevice([&](vector<uint8_t> &r) {
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

        if ((not usbComm.WiringOk())
            or (not interfaceManager.WiringOk())
            or ((high_resolution_clock::now() - lastProcessFunctionExecution) < TIMEOUT)) {
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

        interfaceManager->syncWithDevice([&](vector<uint8_t> &r) {
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

//template<>
//struct jsonType<int> {
//    static const Json::ValueType value = Json::intValue;
//
//    static void execute(const Json::Value &key, function<void(int)> &setter) {
//        setter(key.asInt());
//    }
//};

void bridge::BridgeProcessor::tryAssignInt(const rapidjson::Value &key, function<void(int)> setter) {

    if (not key.IsInt()) {
        //  logger.error("Value for " + " is in invalid format.");
        return;
    }

    setter(key.GetInt());
}

void bridge::BridgeProcessor::tryAssignBool(const rapidjson::Value &key, function<void(bool)> setter) {
    if (not key.IsBool()) {
        // logger.error("Value for " + key.toStyledString() + " is in invalid format.");
        return;
    }

    setter(key.GetBool());
}

void bridge::BridgeProcessor::tryAssignDirection(const rapidjson::Value &key, function<void(Direction)> setter) {
    try {
        auto dir = stringToDirection(key.GetString());
        setter(dir);
    } catch (runtime_error &e) {
        // logger.error("Value for " + key.toStyledString() + ": " + e.what() + ".");
    }
}

void bridge::BridgeProcessor::parseRequest(rapidjson::Document &r) {
    using namespace std::placeholders;
    // TODO wkładanie requestów do interfejsu

    tryAssignBool(r["ks_en"], std::bind(&Interface::setKillSwitch, &iface(), _1));
    if (r["lcd"].IsString()) {
        iface().setLCDText(r["lcd"].GetString());
    }

    auto fillArm = [&](const char *name, Joint j) {
        tryAssignInt(r["arm"][name]["speed"],
                     bind(&Interface::ArmClass::SingleJoint::setSpeed, &iface().arm[j], _1));

        tryAssignDirection(r["arm"][name]["dir"],
                           bind(&Interface::ArmClass::SingleJoint::setDirection, &iface().arm[j], _1));

        if (!r["arm"][name]["dir"].IsString()) {
            tryAssignInt(r["arm"][name]["pos"],
                         bind(&Interface::ArmClass::SingleJoint::setPosition, &iface().arm[j], _1));
        }
    };

    // TODO arm ogólne ustawienia kalibracja itd.

    auto fillMotor = [&](const char *name, Motor m) {
        tryAssignInt(r["motor"][name]["speed"],
                     bind(&Interface::MotorClass::SingleMotor::setSpeed, &iface().motor[m], _1));

        tryAssignDirection(r["motor"][name]["dir"],
                           bind(&Interface::MotorClass::SingleMotor::setDirection, &iface().motor[m], _1));
    };

    auto fillExpander = [&](const char *name, ExpanderDevice d) {
        tryAssignInt(r["light"][name],
                     bind(&Interface::ExpanderClass::Device::setEnabled, &iface().expander[d], _1));
    };

    tryAssignBool(r["arm"]["b_cal"], [&](bool startCalibration) {
        if (startCalibration) {
            iface().arm.calibrate();
        }
    });

    fillAllDevices(fillArm, fillMotor, fillExpander);
}

void bridge::BridgeProcessor::createReport(minijson::object_writer &r) {

    {
        auto light = r.nested_object("light");
        auto fillExpander = [&](const char *name, ExpanderDevice d) {
            light.write(name, iface().expander[d].isEnabled());
        };

        fillExpander("right", ExpanderDevice::LIGHT_RIGHT);
        fillExpander("left", ExpanderDevice::LIGHT_LEFT);
        fillExpander("camera", ExpanderDevice::LIGHT_CAMERA);

        light.close();
    }

    {
        auto motor = r.nested_object("motor");

        auto fillMotor = [&](const char *name, Motor m) {
            auto specificMotor = motor.nested_object(name);
            specificMotor.write("speed", iface().motor[m].getSpeed());
            specificMotor.write("dir", directionToString(iface().motor[m].getDirection()));
            specificMotor.close();
        };

        fillMotor("left", Motor::LEFT);
        fillMotor("right", Motor::RIGHT);

        motor.close();
    }

    {
        auto arm = r.nested_object("arm");

        auto fillArm = [&](const char *name, Joint j) {
            auto joint = arm.nested_object(name);
            joint.write("speed", iface().arm[j].getSpeed());
            joint.write("pos", iface().arm[j].getPosition());
            joint.write("dir", directionToString(iface().arm[j].getDirection()));
            joint.close();
        };

        fillArm("shoulder", Joint::SHOULDER);
        fillArm("elbow", Joint::ELBOW);
        fillArm("gripper", Joint::GRIPPER);

        arm.write("cal_st", armCalibrationStatusToString(iface().arm.getCalibrationStatus()));
        arm.write("mode", armDriverModeToString(iface().arm.getMode()));

        arm.close();
    }

    {
        auto button = r.nested_array("button");

        auto fillButtons = [&](string name, Button d) {
            if (iface().isButtonPressed(d)) {
                button.write(name);
            }
        };

        fillButtons("up", Button::UP);
        fillButtons("down", Button::DOWN);
        fillButtons("enter", Button::ENTER);

        button.close();
    }

    {
        auto batt = r.nested_object("batt");
        batt.write("volt", iface().getVoltage());
        batt.write("curr", iface().getCurrent());
        batt.close();
    }

    if (iface().isKillSwitchActive()) {
        if (iface().isKillSwitchCausedByHardware()) {
            r.write("ks_stat", "hardware");
        } else {
            r.write("ks_stat", "software");
        }
    } else {
        r.write("ks_stat", "inactive");
    }
}

void bridge::BridgeProcessor::fillAllDevices(
        std::function<void(const char *, Joint)> fillArm,
        std::function<void(const char *, Motor)> fillMotor,
        std::function<void(const char *, ExpanderDevice)> fillExpander) {

    fillExpander("right", ExpanderDevice::LIGHT_RIGHT);
    fillExpander("left", ExpanderDevice::LIGHT_LEFT);
    fillExpander("camera", ExpanderDevice::LIGHT_CAMERA);

    fillMotor("left", Motor::LEFT);
    fillMotor("right", Motor::RIGHT);

    fillArm("shoulder", Joint::SHOULDER);
    fillArm("elbow", Joint::ELBOW);
    fillArm("gripper", Joint::GRIPPER);
}