#ifndef _USB_COMMANDS_
#define _USB_COMMANDS_

#include <stdint.h>

#include "arm_driver-commands.hpp"
#include "motor_driver-commands.hpp"

namespace USBCommands {

enum Request
	: uint8_t {
		BRIDGE_LCD_SET = 200,
	BRIDGE_GET_STATE,
	BRIDGE_SET_KILLSWITCH,
	BRIDGE_RESET_DEVICE,
	MOTOR_DRIVER_SET,
	MOTOR_DRIVER_GET,
	ARM_DRIVER_SET,
	ARM_DRIVER_GET,
	ARM_DRIVER_GET_GENERAL_STATE,
	EXPANDER_SET,
	EXPANDER_GET,
	MESSAGE_END = 222
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

	bool killSwitchCausedByHardware;
	KillSwitch killSwitch;
}__attribute__((packed));

static_assert(sizeof(State) == 9, "Struct State has wrong alignment.");
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
	:uint8_t {BRAKE = 100, CALIBRATE
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
