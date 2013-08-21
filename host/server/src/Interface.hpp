/*
 * Interface.hpp
 *
 *  Created on: 19-08-2013
 *      Author: michal
 */

#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_

#include <string>
#include <map>
#include <memory>

#include <boost/noncopyable.hpp>

namespace bridge {

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
		UP, DOWN, ENTER
	};

	/**
	 * Joints of the arm driver.
	 */
	enum class Joint {
		ELBOW = 'e', WRIST = 'w', GRIPPER = 'g', SHOULDER = 's'
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

	/**
	 * Devices connected to the I2C expander. The enum numbers must match the bit number of the pin the device
	 * is connected.
	 */
	enum class ExpanderDevice {
		LIGHT_CAMERA = 3, LIGHT_LEFT = 2, LIGHT_RIGHT = 1
	};

	struct Implementation;

	/**
	 * This class provides full interface to SHARK device. TODO write documentation for interface class
	 */
	class Interface: boost::noncopyable {
	public:
		Interface();
		virtual ~Interface();

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
		std::string getLCDText();

		/**
		 * Sends the text to the on-board LCD display. Up to 32 characters will be displayed, the rest will
		 * be ignored. Every time you send the new text the old one will be cleared. You can't simply append
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
		bool isKillSwitchActive();

		/**
		 * Returns true if the given button is pressed.
		 */
		bool isButtonPressed(Button button);

		// TODO documentation and rename
		void sendChanges();

	private:
		Implementation *impl;

	public:

		class MotorClass {
		public:
			class SingleMotor: boost::noncopyable {
			public:
				unsigned int getSpeed();
				void setSpeed(unsigned int speed);

				/**
				 * Returns the actual PWM power delivered to the motor.
				 */
				unsigned int getPower();

				Direction getDirection();
				void setDirection(Direction direction);

				Motor getMotor() {
					return motor;
				}
			private:
				SingleMotor(Implementation* impl, Motor motor) {
					this->motor = motor;
					this->impl = impl;
				}
				Motor motor;
				Implementation *impl;

				friend class MotorClass;
			};

			SingleMotor& operator[](Motor motor) {
				return *motors[motor];
			}

			SingleMotor& m(Motor motor) {
				return *motors[motor];
			}

			void brake();
		private:
			MotorClass(Implementation* impl) {
				motors[Motor::LEFT] = std::shared_ptr<SingleMotor>(new SingleMotor(impl, Motor::LEFT));
				motors[Motor::RIGHT] = std::shared_ptr<SingleMotor>(new SingleMotor(impl, Motor::RIGHT));
			}

			std::map<Motor, std::shared_ptr<SingleMotor>> motors;

			friend class Interface;
		};

		class ArmClass: boost::noncopyable {
		public:
			class SingleJoint: boost::noncopyable {
			public:
				unsigned int getSpeed();
				void setSpeed(unsigned int speed);

				Direction getDirection();
				void setDirection(Direction direction);

				Joint getJoint() {
					return joint;
				}

				unsigned int getPosition();
				void setPosition(unsigned int position);
			private:
				SingleJoint(Implementation* impl, Joint joint) {
					this->joint = joint;
					this->impl = impl;
				}

				Joint joint;
				Implementation *impl;

				friend class ArmClass;
			};

			SingleJoint& operator[](Joint joint) {
				return *joints[joint];
			}

			void brake();

			void calibrate();

			bool isCalibrated();

			ArmDriverMode getMode();

		private:
			ArmClass(Implementation* impl) {
				this->impl = impl;

				for (auto j : { Joint::ELBOW, Joint::GRIPPER, Joint::SHOULDER, Joint::WRIST }) {
					joints[j] = std::shared_ptr<SingleJoint>(new SingleJoint(impl, j));
				}
			}

			std::map<Joint, std::shared_ptr<SingleJoint>> joints;
			Implementation* impl;

			friend class Interface;
		};

		class ExpanderClass: boost::noncopyable {
		public:
			class Device: boost::noncopyable {
			public:
				void setEnabled(bool enabled);
				bool isEnabled();
			private:
				Device(Implementation* impl, ExpanderDevice device) {
					this->device = device;
					this->impl = impl;
				}
				ExpanderDevice device;
				Implementation* impl;

				friend class ExpanderClass;
			};

			Device& operator[](ExpanderDevice device) {
				return *devices[device];
			}

		private:
			ExpanderClass(Implementation* impl) {
				for (auto d : { ExpanderDevice::LIGHT_CAMERA, ExpanderDevice::LIGHT_LEFT, ExpanderDevice::LIGHT_RIGHT }) {
					devices[d] = std::shared_ptr<Device>(new Device(impl, d));
				}
			}

			std::map<ExpanderDevice, std::shared_ptr<Device>> devices;

			friend class Interface;
		};
	public:
		MotorClass& motor;
		ArmClass& arm;
		ExpanderClass& expander;
	};
}
/* namespace bridge */
#endif /* INTERFACE_HPP_ */
