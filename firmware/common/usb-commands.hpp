#ifndef _USB_COMMANDS_
#define _USB_COMMANDS_

#include <stdint.h>

#include "arm_driver-commands.hpp"
#include "motor_driver-commands.hpp"

namespace USBCommands {

	enum Request
			: uint8_t {
		BRIDGE_LCD_SET = 200,
		BRIDGE_GET_STATE = 201,
		BRIDGE_SET_KILLSWITCH = 202,
		BRIDGE_RESET_DEVICE = 203,
		MOTOR_DRIVER_SET = 204,
		MOTOR_DRIVER_GET = 205,
		ARM_DRIVER_SET = 206,
		ARM_DRIVER_GET = 207,
		ARM_DRIVER_GET_GENERAL_STATE = 208,
		EXPANDER_SET = 209,
		EXPANDER_GET = 210,
		MESSAGE_END = 222
	};

	namespace bridge {
		enum KillSwitch
				: uint8_t {
			INACTIVE, ACTIVE
		};

		struct State {
			uint16_t rawVoltage;
			uint16_t rawCurrent;

			bool buttonUp;
			bool buttonDown;
			bool buttonEnter;

			bool killSwitchCausedByHardware;
			KillSwitch killSwitch;
		}__attribute__((packed));

		static_assert(sizeof(State) == 9, "Struct State has wrong alignment.");

		constexpr double BATTERY_VOLTAGE_FACTOR2(double R_up, double R_down, double V_ref, double factor = 1.0) {
			return (V_ref / 1023.0 * (R_up + R_down) / R_down) * factor;
		}

		const double VOLTAGE_FACTOR = BATTERY_VOLTAGE_FACTOR2(8200, 1000, 2.56, 0.997263087);
		const double CURRENT_FACTOR = 0.026;
	}

	namespace motor_commands = motor;
	namespace arm_commands = arm;

	namespace motor {
		struct SpecificMotorState {
			motor_commands::Motor motor;
			motor_commands::Direction direction;
			uint8_t speed;
		};

		static_assert(sizeof(SpecificMotorState) == 3, "Struct SpecificMotorState has wrong alignment.");

	}

	namespace arm {

		enum AdditionalCommands
				: uint8_t {
			BRAKE = 100, CALIBRATE
		};

		struct GeneralState {
			bool isCalibrated;
			arm_commands::Mode mode;
		};

		static_assert(sizeof(GeneralState) == 2, "Struct GeneralState has wrong alignment.");

		struct JointState {
			arm_commands::Motor motor;
			arm_commands::Direction direction;
			uint8_t speed;

			bool setPosition;
			uint8_t position;
		};

		static_assert(sizeof(JointState) == 5, "Struct JointState has wrong alignment.");
	}
}

#endif
