/*
 #  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
 #  module bridge - converter between RS232 and I2C main bus
 #  Michał Słomkowski 2011, 2012
 #  www.slomkowski.eu m.slomkowski@gmail.com
 */

#include "global.h"
#include <util/delay.h>

#include "menu.h"
#include "lcd.h"
#include "buttons.h"
#include "motors.h"
#include "analog.h"
#include "i2c_expander.h"
#include "arm.h"

// battery configuration
#define BATT_VOLT_FAC 71

// testing speed
#define MOTOR_SPEED 3

#define ARM_SPEED 5

#define ARM_SHOULDER_SPEED ARM_SPEED
#define ARM_ELBOW_SPEED ARM_SPEED
#define ARM_WRIST_SPEED ARM_SPEED
#define ARM_GRIPPER_SPEED ARM_SPEED

typedef enum {
	INACTIVE = 1, MAIN_MENU, ARM_MENU, MOTOR_MENU, EXP_MENU, ARM_MENU_WORK, MOTOR_MENU_WORK
} MENU_STATE;

static MENU_STATE mainMenuSelector;
static int8_t subMenuSelector = ARM_MENU;

void menuInit() {
	mainMenuSelector = INACTIVE;
}

void menuSetToMain() {
	subMenuSelector = ARM_MENU;
	mainMenuSelector = INACTIVE;
}

static void batteryDisplay() {
	static char text[] = "Batt: xx.xV\n";

	uint8_t index;
	uint16_t volt = (voltage / 4) * BATT_VOLT_FAC / 10;

	if (volt % 10 >= 5) volt = volt / 10 + 1;
	else volt /= 10;

	// aka itoa
	for (index = 9; index >= 6; index--)
		if (index != 8) {
			text[index] = volt % 10 + '0';
			volt /= 10;
		}

	lcd_clrscr();
	lcd_puts(text);
}

static uint8_t armButtonsControlAndPos(uint8_t motor, uint8_t speed) {
	arm_set_speed(motor, speed);
	if (buttonPressed(BUTTON_UP)) arm_set_direction(motor, arm::MOTOR_FORWARD);
	else if (buttonPressed(BUTTON_DOWN)) arm_set_direction(motor, arm::MOTOR_BACKWARD);
	else arm_set_direction(motor, arm::MOTOR_STOP);

	return arm_get_position(motor);
}

static void motorButtonsControl(uint8_t motor, uint8_t speed) {
	motor_set_speed(motor, speed);
	if (buttonPressed(BUTTON_UP)) motor_set_direction(motor, motor::FORWARD);
	else if (buttonPressed(BUTTON_DOWN)) motor_set_direction(motor, motor::BACKWARD);
	else motor_set_direction(motor, motor::STOP);
}

static void checkSelector(uint8_t lower, uint8_t upper) {
	if (buttonPressed(BUTTON_UP)) subMenuSelector++;
	else if (buttonPressed(BUTTON_DOWN)) subMenuSelector--;

	if (subMenuSelector < lower) subMenuSelector = upper;
	else if (subMenuSelector > upper) subMenuSelector = lower;
}

static uint8_t armcalibrationDone = 0, busIsActive = 0, armPosition = 0, motorSpeed = 0;

static void checkEnterButtonMenu(uint8_t exitId, uint8_t workMenuId) {
	if (buttonPressed(BUTTON_ENTER)) {
		if (subMenuSelector == exitId) {
			mainMenuSelector = MAIN_MENU;
			subMenuSelector = 1;
		} else {
			mainMenuSelector = (MENU_STATE) workMenuId;
			/*emergencyStopDisable();
			 armcalibrationDone = 0;
			 busIsActive = 1;
			 _delay_ms(100);*/
		}
	}
}

static void checkEnterButtonWorkMenu(uint8_t upperMenu) {
	if (buttonPressed(BUTTON_ENTER)) {
		busIsActive = 0;
		//emergencyStopPerform();
		mainMenuSelector = (MENU_STATE) upperMenu;
	}
}

// main function
void menuCheckButtons() {
	// lots of static variables, I know
	static uint16_t counter; // this is used by software counter, switch is called when counter overflows

	static char armPositionText[] = "Position: XXX"; // X fields are replaced by digits

	uint8_t i;

	// this is to check for hardware emergency button, very frequently
	if (busIsActive && emergencyIsStopped()) {
		busIsActive = 0;
		emergencyStopPerform();
		mainMenuSelector = INACTIVE;
		lcd_clrscr();
		lcd_puts_P("INACTIVE\nEMERGENCY BUTTON");
	}

	if (counter == 0) counter = MENU_REFRESH_NUMBER;
	else {
		counter--;
		return;
	}

	// this switch is called much more rarely, because it interacts with the user and LCD display can't be refreshed so quickly
	switch (mainMenuSelector) {
	case INACTIVE:
		if (buttonPressed(BUTTON_ENTER) || buttonPressed(BUTTON_UP) || buttonPressed(BUTTON_DOWN)) {
			mainMenuSelector = MAIN_MENU;
		}
		break;
	case MAIN_MENU:
		// battery state
		batteryDisplay();
		emergencyStopPerform();
		switch (subMenuSelector) {
		case ARM_MENU:
			lcd_puts_P("< ARM DRIVER >");
			break;
		case MOTOR_MENU:
			lcd_puts_P("< MOTOR DRIVER >");
			break;
		case EXP_MENU:
			lcd_puts_P("< I2C EXPANDER >");
			break;
		}
		checkSelector(ARM_MENU, EXP_MENU);

		if (buttonPressed(BUTTON_ENTER)) {
			mainMenuSelector = (MENU_STATE) subMenuSelector;

			emergencyStopDisable();
			busIsActive = 1;
			_delay_ms(100);

			if (mainMenuSelector == EXP_MENU) {
				//emergencyStopDisable();
				//busIsActive = 1;
				//_delay_ms(100);
				subMenuSelector = 8;
			} else subMenuSelector = 1;
		}
		break;
	case ARM_MENU:
		lcd_clrscr();
		lcd_puts_P("Arm driver:\n");
		switch (subMenuSelector) {
		case 1:
			lcd_puts_P("< CALIBRATE >");
			break;
		case 2:
			lcd_puts_P("< SHOULDER >");
			break;
		case 3:
			lcd_puts_P("< ELBOW >");
			break;
		case 4:
			lcd_puts_P("< WRIST >");
			break;
		case 5:
			lcd_puts_P("< GRIPPER >");
			break;
		case 6:
			lcd_puts_P("< EXIT >");
			break;
		}

		checkSelector(1, 6);
		checkEnterButtonMenu(6, ARM_MENU_WORK);
		if (subMenuSelector == 1) // calibrate
		armcalibrationDone = 0;

		break;
	case ARM_MENU_WORK:
		lcd_clrscr();
		lcd_puts_P("ARM: ");
		// tu właściwe sterowanie ramieniem

		switch (subMenuSelector) {
		case 1: // calibrate
			lcd_puts_P("calibrate\n");
			if (!armcalibrationDone) {
				arm_calibrate();
				armcalibrationDone = 1;
			}
			// give time for reaction
			_delay_ms(BUTTON_DEBOUNCE_TIME);

			if (arm_is_calibrated()) lcd_puts_P("finished: YES");
			else lcd_puts_P("finished: NO");
			break;
		case 2: // shoulder
			lcd_puts_P("shoulder\n");
			armPosition = armButtonsControlAndPos(arm::MOTOR_SHOULDER, ARM_SHOULDER_SPEED);
			break;
		case 3:
			lcd_puts_P("elbow\n");
			armPosition = armButtonsControlAndPos(arm::MOTOR_ELBOW, ARM_ELBOW_SPEED);
			break;
		case 4:
			lcd_puts_P("wrist\n");
			armPosition = armButtonsControlAndPos(arm::MOTOR_WRIST, ARM_WRIST_SPEED);
			break;
		case 5:
			lcd_puts_P("gripper\n");
			armPosition = armButtonsControlAndPos(arm::MOTOR_GRIPPER, ARM_GRIPPER_SPEED);
			break;
		}
		// display position
		if (subMenuSelector != 1) // when it's not calibrate menu
			{
			for (i = 12; i > 9; i--) {
				armPositionText[i] = (armPosition % 10 + '0');
				armPosition /= 10;
			}
			lcd_puts(armPositionText);
		}

		checkEnterButtonWorkMenu(ARM_MENU);

		break;
	case MOTOR_MENU:
		lcd_clrscr();
		lcd_puts_P("Motor driver:\n");
		switch (subMenuSelector) {
		case 1:
			lcd_puts_P("< LEFT WHEEL >");
			break;
		case 2:
			lcd_puts_P("< RIGHT WHEEL >");
			break;
		case 3:
			lcd_puts_P("< BOTH WHEELS >");
			break;
		case 4:
			lcd_puts_P("< EXIT >");
			break;
		}

		checkSelector(1, 4);
		checkEnterButtonMenu(4, MOTOR_MENU_WORK);

		break;
	case MOTOR_MENU_WORK:
		lcd_clrscr();
		switch (subMenuSelector) {
		case 1:
			lcd_puts_P("MOTOR: left");
			motorButtonsControl(MOTOR_LEFT, MOTOR_SPEED);
			break;
		case 2:
			lcd_puts_P("MOTOR: right");
			motorButtonsControl(MOTOR_RIGHT, MOTOR_SPEED);
			break;
		case 3:
			lcd_puts_P("MOTOR: both");
			motorButtonsControl(MOTOR_LEFT, MOTOR_SPEED);
			motorButtonsControl(MOTOR_RIGHT, MOTOR_SPEED);
			break;
		}

		static char speedText[] = "\nL: xxx, R: xxx";
		motorSpeed = motor_get_speed(MOTOR_LEFT);
		for (i = 6; i > 3; i--) {
			speedText[i] = (motorSpeed % 10 + '0');
			motorSpeed /= 10;
		}
		motorSpeed = motor_get_speed(MOTOR_RIGHT);
		for (i = 14; i > 11; i--) {
			speedText[i] = (motorSpeed % 10 + '0');
			motorSpeed /= 10;
		}
		lcd_puts(speedText);

		checkEnterButtonWorkMenu(MOTOR_MENU);

		break;
	case EXP_MENU:
		lcd_clrscr();
		lcd_puts_P("EXPANDER\nDevice: ");
		if (subMenuSelector == 8) lcd_puts_P("OFF");
		else lcd_putc('0' + subMenuSelector);

		// send it to expander
		i2c_exp_set_value(1 << subMenuSelector);

		checkSelector(0, 8);

		checkEnterButtonWorkMenu(MAIN_MENU);
		break;
	}
}

