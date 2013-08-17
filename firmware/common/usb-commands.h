#ifndef _USB_COMMANDS_
#define _USB_COMMANDS_

#include <stdint.h>
#include "arm_driver-commands.h"
#include "motor_driver-commands.h"

namespace USBCommands {

	enum USBRequest
		:uint8_t {
			USB_READ = 1, USB_WRITE
	};

	enum Request
		: uint8_t {
			BRIDGE_LCD_SET = 1,
		BRIDGE_GET_STATE,
		BRIDGE_SET_KILLSWITCH,
		MOTOR_DRIVER_SET,
		MOTOR_DRIVER_GET,
		ARM_DRIVER_SET,
		ARM_DRIVER_GET,
		EXPANDER_SET,
		EXPANDER_GET
	};

	namespace bridge {
		enum KillSwitch
			:uint8_t {
				INACTIVE, ACTIVE
		};

		struct State {
			uint16_t rawVoltage;
			uint16_t rawCurrent;

			bool buttonUp;
			bool buttonDown;
			bool buttonEnter;

			KillSwitch killSwitch;
		};
	}

	namespace motor_commands = motor;
	namespace arm_commands = arm;

	namespace motor {
		struct SpecificMotorState {
			motor_commands::Motor motor;
			motor_commands::Direction direction;
			uint8_t speed;
		};
	}

	namespace arm {
		struct GeneralState {
			bool isCalibrated;
			arm_commands::Mode dd;
		};

		struct JointState {
			arm_commands::Motor motor;
			arm_commands::Direction direction;
			uint8_t speed;

			bool setPosition;
			uint8_t position;
		};
	}
}

#endif
