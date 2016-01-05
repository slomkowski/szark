#pragma once

#include "DataHolder.hpp"

#include <boost/noncopyable.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <string>
#include <map>
#include <memory>

namespace common {
    namespace bridge {
        constexpr uint8_t MOTOR_DRIVER_MAX_SPEED = 11;

        constexpr unsigned int ARM_DRIVER_MAX_SPEED = 255;

        /**
        * Direction used by motor driver and arm driver.
        */
        enum class Direction {
            STOP, FORWARD, BACKWARD
        };
        /**
        * Buttons located on the bridge board.
        */
        enum class Button {
            UP,
            DOWN, ENTER
        };
        /**
        * Joints of the arm driver.
        */
        enum class Joint {
            ELBOW = 'e', GRIPPER = 'g', SHOULDER = 's'
        };
        /**
        * Motors of the motor driver.
        */
        enum class Motor {
            RIGHT = 'r', LEFT = 'l'
        };
        /**
        * Modes of operation of the arm driver. Default is directional mode. You set the direction
        * and the joints moves respectively. If you send setPosition() command, the driver goes to positional
        * mode. You can go back to directional mode by sending setDirection() command. If you send calibrate()
        */
        enum class ArmDriverMode {
            DIRECTIONAL, POSITIONAL, CALIBRATING
        };

        enum class ArmCalibrationStatus {
            NONE,
            IN_PROGRESS, DONE
        };
        /**
        * Devices connected to the I2C expander. The enum numbers must match the bit number of the pin the device
        * is connected.
        */
        enum class ExpanderDevice
                : uint8_t {
            LIGHT_CAMERA = 4, LIGHT_LEFT = 2, LIGHT_RIGHT = 3
        };

        typedef std::pair<const std::string, DataHolder> ValueType;
        typedef boost::interprocess::allocator<ValueType, boost::interprocess::fixed_managed_shared_memory::segment_manager>
                ShmemAllocator;
        typedef boost::interprocess::map<std::string, DataHolder, std::less<std::string>, ShmemAllocator> SharedRequestMap;
        typedef std::map<const std::string, DataHolder> RequestMap;

        typedef boost::interprocess::offset_ptr<SharedRequestMap> SharedRequestMapPtr;

        typedef boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> SharedScopedMutex;

        class IExternalDevice : boost::noncopyable {
        private:
            SharedRequestMapPtr requests;

            IExternalDevice(SharedRequestMapPtr requests)
                    : requests(requests) { }

            /**
            * This method is called when the device returns data requested by sending getters. Each device like arm driver
            * and motor driver classes implement it.
            * @param request USB request identifier.
            * @param data pointer to payload of the request. The concrete method implementation must cast it to its own structure.
            * @return number of bytes taken by the message without request. If the request doesn't match,
            * return 0.
            */
            virtual unsigned int updateFields(USBCommands::Request request, uint8_t *data) = 0;

            /**
            * This method is called when kill switch is activated. It should set internal variables to 0, STOP etc. because
            * the external device is made into reset state.
            */
            virtual void onKillSwitchActivated() = 0;

            /**
             * This is a convenience method to insert/update elements into requests map.
             */
            void insertRequest(std::string key, DataHolder value);

        public:
            virtual ~IExternalDevice() = default;

            friend class Interface;
        };

        /**
        * This class provides full interface to SHARK device. TODO write documentation for interface class
        */
        class Interface : public IExternalDevice {
        public:
            Interface(SharedRequestMapPtr requestMap);

            virtual ~Interface();

            /**
             * Returns true if Interface object is constructed. Destructor sets it to false.
             * This can be useful when Interface is stored in shared memory. Other processes
             * have the way to known whether the object was destroyed.
             */
            bool isObjectActive() {
                return objectActive;
            }

            /**
            * Returns actual main power supply voltage in volts.
            */
            double getVoltage();

            /**
            * Returns actual overall current draw from the battery in amperes.
            */
            double getCurrent();

            /**
            * Returns the text previously sent to on-board LCD display. It doesn't return actual LCD text
            * since bridge doesn't support this functionality. If you haven't send any text previously,
            * empty string will be returned.
            */
            std::string getLCDText() {
                return lcdText;
            }

            /**
            * Sends the text to the on-board LCD display. Up to 32 characters will be displayed, the rest will
            * be ignored. Every time you send the new text the old one will be cleared. You can't srequestsy append
            * to the existing one.
            */
            void setLCDText(std::string text);

            /**
            * Set the state of the kill switch feature. Activating kill switch forces immediate stop of all devices
            * connected to local I2C bus. The device's reset pin will be held active until you manually reactivate it.
            * The kill switch may be activated by hardware by pressing emergency stop button on the SHARK device.
            * In this case you have to reactivate it as well. It will fail if the kill switch is still toggled.
            */
            void setKillSwitch(bool active);

            /**
            * Returns the kill switch state.
            */
            bool isKillSwitchActive() {
                return killSwitchActive;
            }

            /**
            * Returns true if the kill switch was activated by pressing it.
            */
            bool isKillSwitchCausedByHardware() {
                return killSwitchCausedByHardware;
            }

            /**
            * Returns true if the given button is pressed.
            */
            bool isButtonPressed(Button button);

        private:
            bool objectActive;

            /**
            * If the kill switch is requested by the user or detected by hardware, this function should be called.
            * It basically resets interface getter structures to match the actual state.
            */
            void updateStructsWhenKillSwitchActivated();

            const std::string KILLSWITCH_STRING = "killswitch";

            boost::circular_buffer<unsigned int> rawVoltage;
            boost::circular_buffer<unsigned int> rawCurrent;

            std::string lcdText;

            std::map<Button, bool> buttons;

            bool killSwitchActive;
            bool killSwitchCausedByHardware;

            std::vector<IExternalDevice *> extDevListeners;

            unsigned int updateFields(USBCommands::Request request, uint8_t *data);

            void onKillSwitchActivated();

        public:

            class MotorClass : boost::noncopyable {
            public:
                class SingleMotor : public IExternalDevice {
                public:
                    /**
                    * Returns the programmed motor speed.
                    * @return speed value from 0 to 12
                    */
                    unsigned int getSpeed() {
                        return programmedSpeed;
                    }

                    /**
                    * Sets the motor speed.
                    * @param speed value from 0 to 12
                    */
                    void setSpeed(unsigned int speed);

                    /**
                    * Returns the actual PWM power delivered to the motor.
                    * @return power in range 0 to 255.
                    */
                    unsigned int getPower() {
                        return power;
                    }

                    /**
                    * Returns actual motor's direction.
                    * @return Direction::FORWARD, Direction::BACKWARD or Direction::STOP.
                    */
                    Direction getDirection() {
                        return direction;
                    }

                    /**
                    * Sets the motor's direction.
                    * @param direction one of Direction::FORWARD, Direction::BACKWARD or Direction::STOP.
                    */
                    void setDirection(Direction dir);

                private:
                    SingleMotor(SharedRequestMapPtr requests, Motor motor)
                            : IExternalDevice(requests),
                              motor(motor) {
                        direction = Direction::STOP;
                        programmedSpeed = 0;
                        power = 0;
                    }

                    std::string getKey() {
                        return std::string("motor_") + std::to_string((int) motor);
                    }

                    unsigned int updateFields(USBCommands::Request request, uint8_t *data);

                    void createMotorState();

                    void onKillSwitchActivated();

                    void initStructure();

                    Motor motor;

                    Direction direction;
                    uint8_t programmedSpeed;

                    uint8_t power;

                    friend class MotorClass;
                };

                SingleMotor left;
                SingleMotor right;

                /**
                * Convenience operator for getting one of the available motors.
                * @param motor
                * @return reference to motor class providing manipulation methods.
                */
                SingleMotor &operator[](Motor motor) {
                    return *motors.at(motor);
                }

                /**
                * Stops all the motors.
                */
                void brake();

            private:
                MotorClass(SharedRequestMapPtr requests)
                        : left(requests, Motor::LEFT),
                          right(requests, Motor::RIGHT) {
                    motors = {
                            {Motor::LEFT,  &left},
                            {Motor::RIGHT, &right}
                    };
                }

                std::map<Motor, SingleMotor *> motors;

                friend class Interface;
            };

            class ArmClass : public IExternalDevice {
            public:
                class SingleJoint : public IExternalDevice {
                public:
                    unsigned int getSpeed() {
                        return speed;
                    }

                    void setSpeed(unsigned int speed);

                    Direction getDirection() {
                        return direction;
                    }

                    void setDirection(Direction direction);

                    unsigned int getPosition() {
                        return position;
                    }

                    /**
                    * Enables arm position mode and sets the position of the joint. Each joint has limited number of positions.
                    * If the position is limited to the maximal allowed for the given joint.
                    * @param position value of position in the valid range.
                    */
                    void setPosition(unsigned int position);

                private:
                    SingleJoint(SharedRequestMapPtr requests,
                                Joint joint,
                                ArmCalibrationStatus &calibrationStatus)
                            : IExternalDevice(requests),
                              joint(joint),
                              calibrationStatus(calibrationStatus) {
                        speed = 0;
                        direction = Direction::STOP;
                        position = 0;
                        programmedPosition = 0;

                        settingPosition = false;
                    }

                    unsigned int updateFields(USBCommands::Request request, uint8_t *data);

                    void onKillSwitchActivated();

                    void initStructure();

                    void createJointState();

                    std::string getKey() {
                        return std::string("arm_") + std::to_string((int) joint);
                    }

                    Joint joint;
                    ArmCalibrationStatus &calibrationStatus;

                    uint8_t speed;
                    Direction direction;
                    uint8_t position;
                    uint8_t programmedPosition;
                    bool settingPosition;

                    friend class ArmClass;
                };

                SingleJoint shoulder;
                SingleJoint elbow;
                SingleJoint gripper;

                SingleJoint &operator[](Joint joint) {
                    return *joints.at(joint);
                }

                void brake();

                void calibrate();

                ArmCalibrationStatus getCalibrationStatus() {
                    return calibrationStatus;
                }

                ArmDriverMode getMode() {
                    return mode;
                }

            private:
                ArmClass(SharedRequestMapPtr requests)
                        : IExternalDevice(requests),
                          shoulder(requests, Joint::SHOULDER, calibrationStatus),
                          elbow(requests, Joint::ELBOW, calibrationStatus),
                          gripper(requests, Joint::GRIPPER, calibrationStatus) {

                    joints = {
                            {Joint::ELBOW,    &elbow},
                            {Joint::GRIPPER,  &gripper},
                            {Joint::SHOULDER, &shoulder}
                    };

                    mode = ArmDriverMode::DIRECTIONAL;
                    calibrationStatus = ArmCalibrationStatus::NONE;
                }

                unsigned int updateFields(USBCommands::Request request, uint8_t *data);

                void onKillSwitchActivated();

                std::map<Joint, SingleJoint *> joints;

                ArmDriverMode mode;

                ArmCalibrationStatus calibrationStatus;

                friend class Interface;
            };

            class ExpanderClass : public IExternalDevice {
            public:
                class Device : boost::noncopyable {
                public:
                    void setEnabled(bool enabled);

                    bool isEnabled();

                private:
                    Device(SharedRequestMapPtr requests,
                           ExpanderClass *expanderClass,
                           ExpanderDevice device)
                            : requests(requests),
                              device(device),
                              expanderClass(expanderClass) { }

                    SharedRequestMapPtr requests;

                    ExpanderDevice device;
                    ExpanderClass *expanderClass;

                    friend class ExpanderClass;
                };

                Device lightCamera;
                Device lightLeft;
                Device lightRight;

                Device &operator[](ExpanderDevice device) {
                    return *devices.at(device);
                }

            private:
                ExpanderClass(SharedRequestMapPtr requests)
                        : IExternalDevice(requests),
                          lightCamera(requests, this, ExpanderDevice::LIGHT_CAMERA),
                          lightLeft(requests, this, ExpanderDevice::LIGHT_LEFT),
                          lightRight(requests, this, ExpanderDevice::LIGHT_RIGHT) {
                    devices = {
                            {ExpanderDevice::LIGHT_CAMERA, &lightCamera},
                            {ExpanderDevice::LIGHT_LEFT,   &lightLeft},
                            {ExpanderDevice::LIGHT_RIGHT,  &lightRight}
                    };
                }

                unsigned int updateFields(USBCommands::Request request, uint8_t *data);

                void onKillSwitchActivated();

                uint8_t expanderByte = 0;

                std::map<ExpanderDevice, Device *> devices;

                friend class Interface;
            };

        public:
            ExpanderClass expander;
            MotorClass motor;
            ArmClass arm;

            boost::interprocess::interprocess_mutex mutex;

            SharedRequestMapPtr getRequestMap() {
                return requests;
            }

            /**
            * Fills the interface's structures with data from the device. Because of that the getters like getSpeed()
            * could work.
            * @param getterRequests vector of getter requests.
            * @param deviceResponse response from the device. The responses order must match the request's.
            */
            void updateDataStructures(std::vector<USBCommands::Request> getterRequests,
                                      std::vector<uint8_t> deviceResponse);
        };
    }
}
