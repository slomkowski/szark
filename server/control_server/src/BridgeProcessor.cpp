#define WALLAROO_REMOVE_DEPRECATED

#include "BridgeProcessor.hpp"
#include "Interface.hpp"
#include "convert.hpp"
#include "utils.hpp"

#include <boost/format.hpp>
#include <minijson_reader.hpp>

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

void bridge::BridgeProcessor::parseRequest(std::string &request) {
    using namespace std::placeholders;
    using namespace minijson;

    const_buffer_context ctx(request.c_str(), request.size());

    auto parse_motor = [&](Motor m, const char *k, minijson::value v) {
        try {
            dispatch(k)
            << "s" >> [&] { iface().motor[m].setSpeed(v.as_long()); }
            << "d" >> [&] { iface().motor[m].setDirection(stringToDirection(v.as_string())); }
            << minijson::any >> [&] { minijson::ignore(ctx); };
        } catch (runtime_error &e) {
            logger.error("Cannot set value for motor " + devToString(m) + ": " + e.what() + ".");
        }
    };

    auto parse_arm = [&](Joint j, const char *k, minijson::value v) {
        try {
            bool directionSet = false;
            dispatch(k)
            << "s" >> [&] { iface().arm[j].setSpeed(v.as_long()); }
            << "d" >> [&] {
                iface().arm[j].setDirection(stringToDirection(v.as_string()));
                directionSet = true;
            }
            << "p" >> [&] {
                if (!directionSet) {
                    iface().arm[j].setPosition(v.as_long());
                }
            }
            << minijson::any >> [&] { minijson::ignore(ctx); };
        } catch (runtime_error &e) {
            logger.error("Cannot set value for joint " + devToString(j) + ": " + e.what() + ".");
        }
    };

    parse_object(ctx, [&](const char *k, value v) {
        dispatch(k)
        << "lcd" >> [&] { iface().setLCDText(v.as_string()); }
        << "ks_en" >> [&] { iface().setKillSwitch(v.as_bool()); }
        << "m" >> [&] {
            parse_object(ctx, [&](const char *k, value v) {
                dispatch(k)
                << "l" >> [&] { parse_object(ctx, bind(parse_motor, Motor::LEFT, _1, _2)); }
                << "r" >> [&] { parse_object(ctx, bind(parse_motor, Motor::RIGHT, _1, _2)); }
                << minijson::any >> [&] { minijson::ignore(ctx); };
            });
        }
        << "a" >> [&] {
            parse_object(ctx, [&](const char *k, value v) {
                dispatch(k)
                << "s" >> [&] { parse_object(ctx, bind(parse_arm, Joint::SHOULDER, _1, _2)); }
                << "e" >> [&] { parse_object(ctx, bind(parse_arm, Joint::ELBOW, _1, _2)); }
                << "g" >> [&] { parse_object(ctx, bind(parse_arm, Joint::GRIPPER, _1, _2)); }
                << "b_cal" >> [&] {
                    if (v.as_bool()) {
                        iface().arm.calibrate();
                    }
                }
                << minijson::any >> [&] { minijson::ignore(ctx); };
            });
        }
        << "e" >> [&] {
            parse_object(ctx, [&](const char *k, value v) {
                dispatch(k)
                << "r" >> [&] { iface().expander[ExpanderDevice::LIGHT_RIGHT].setEnabled(v.as_bool()); }
                << "l" >> [&] { iface().expander[ExpanderDevice::LIGHT_LEFT].setEnabled(v.as_bool()); }
                << "cam" >> [&] { iface().expander[ExpanderDevice::LIGHT_CAMERA].setEnabled(v.as_bool()); }
                << minijson::any >> [&] { minijson::ignore(ctx); };
            });
        }

        << minijson::any >> [&] { minijson::ignore(ctx); };
    });
}

void bridge::BridgeProcessor::createReport(minijson::object_writer &r) {

    {
        auto light = r.nested_object("e");
        auto fillExpander = [&](const char *name, ExpanderDevice d) {
            light.write(name, iface().expander[d].isEnabled());
        };

        fillExpander("r", ExpanderDevice::LIGHT_RIGHT);
        fillExpander("l", ExpanderDevice::LIGHT_LEFT);
        fillExpander("cam", ExpanderDevice::LIGHT_CAMERA);

        light.close();
    }

    {
        auto motor = r.nested_object("m");

        auto fillMotor = [&](const char *name, Motor m) {
            auto specificMotor = motor.nested_object(name);
            specificMotor.write("s", iface().motor[m].getSpeed());
            specificMotor.write("d", directionToString(iface().motor[m].getDirection()));
            specificMotor.close();
        };

        fillMotor("l", Motor::LEFT);
        fillMotor("r", Motor::RIGHT);

        motor.close();
    }

    {
        auto arm = r.nested_object("a");

        auto fillArm = [&](const char *name, Joint j) {
            auto joint = arm.nested_object(name);
            joint.write("s", iface().arm[j].getSpeed());
            joint.write("p", iface().arm[j].getPosition());
            joint.write("d", directionToString(iface().arm[j].getDirection()));
            joint.close();
        };

        fillArm("s", Joint::SHOULDER);
        fillArm("e", Joint::ELBOW);
        fillArm("g", Joint::GRIPPER);

        arm.write("cal_st", armCalibrationStatusToString(iface().arm.getCalibrationStatus()));
        arm.write("mode", armDriverModeToString(iface().arm.getMode()));

        arm.close();
    }

    {
        auto button = r.nested_array("btn");

        auto fillButtons = [&](string name, Button d) {
            if (iface().isButtonPressed(d)) {
                button.write(name);
            }
        };

        fillButtons("u", Button::UP);
        fillButtons("d", Button::DOWN);
        fillButtons("e", Button::ENTER);

        button.close();
    }

    {
        auto batt = r.nested_object("b");
        batt.write("u", iface().getVoltage());
        batt.write("i", iface().getCurrent());
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
