/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

// main module

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include "uart.h"
#include "lcd.h"
#include "i2c.h"
#include "buttons.h"
#include "analog.h"
#include "motors.h"
#include "i2c_expander.h"
#include "arm.h"
#include "menu.h"
#include "usbdrv.h"


volatile uint8_t enabled = 0;

static volatile uint8_t lcd_modified = 0;
static char lcd_text[LCD_ROWS * LCD_COLUMNS + 1];

PROGMEM const uint8_t usbHidReportDescriptor[22] = { /* USB report descriptor */
0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
	0x09, 0x01,                    // USAGE (Vendor Usage 1)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x00,                    //   USAGE (Undefined)
	0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
	0xc0                           // END_COLLECTION
	};
/* The descriptor above is a dummy only, it silences the drivers. The report
 * it describes consists of one byte of undefined data.
 * We don't transfer our data through HID reports, we use custom requests
 * instead.
 */

int main()
{
	uint8_t i;


	//buttons_init();
	lcd_init();
	analog_init();
	i2c_init();
	menuInit();

#if WATCHD0G_ENABLE
	wdt_enable(WDTO_1S);	// 1 second
#endif

	emergencyStopPerform();
	enabled = 0;

	lcd_clrscr();
	lcd_puts_P("SZARK-Loading...\n 2012 Slomkowski"); // hello message

	sei(); // interrupts activation

	// wait 250 ms
	_delay_ms(250);
	usbDeviceConnect();

	// buttons & emergency stop loop
	while(1)
	{
		usbPoll();

		if(enabled)
		{
			// if EXTERNAL STOP was performed
			if(emergencyIsStopped() && enabled)
			/* <-- why double 'enabled' checking? It's a bit tricky, prevents from invoking code below when software
				stop is performed, which would occur sometimes otherwise */
			{
				emergencyStopPerform();
				enabled = 0;

				// send it six times -- just for sure
				for(i = 0; i < 6; i++) uart_putc(RS_STOP_BUS);

				lcd_clrscr();
				lcd_puts_P("INACTIVE\nEMERGENCY BUTTON");
			}
		}
		else menuCheckButtons(); // menu is active only when bridge is disabled

		if(lcd_modified)
		{
			// this is done here in order not to block the RS interrupt with that
			lcd_clrscr();
			lcd_puts(lcd_text);
			lcd_modified = 0;
		}
#if WATCHD0G_ENABLE
		wdt_reset();
#endif
	}
}

// intercepting UART interrupt
ISR(USART_RXC_vect)
{
	char command1, command2, command3;
	uint8_t lcd_index;

	command1 = uart_getc();

	// these are independent from the state of the device, can be always performed
	switch(command1)
	{
		case RS_BATTERY:
			uart_putc(RS_BATTERY);
			// measure
			uart_putc((uint8_t)(0xff & voltage));
			uart_putc((uint8_t)(0xff & (voltage >> 8)));
			uart_putc((uint8_t)(0xff & current));
			uart_putc((uint8_t)(0xff & (current >> 8)));
			break;
		case RS_LCD_WRITE:
			lcd_index = 0;
			while(1)
			{
				command2 = uart_getc();
				if((command2 == '~') || (command2 == '\0') || (lcd_index == LCD_ROWS * LCD_COLUMNS)) break;
				else lcd_text[lcd_index++] = command2;
			}
			lcd_text[lcd_index] = '\0'; // finish with 0
			lcd_modified = 1; // set flag
#if SET_COMMAND_ACK
			uart_putc(RS_LCD_WRITE);
#endif
			break;
	};

	// this is only to I2C commands, other are 'enabled'-independent
	if(enabled) switch(command1) // main command, majuskule
	{
		case RS_MOTOR:
			command2 = uart_getc(); // command for motor driver
			switch(command2)
			{
				case CHAR_MOTOR_SET_SPEED:
				case CHAR_MOTOR_SET_DIRECTION:
					command1 = uart_getc(); // motor
					command3 = uart_getc(); // value
					motor_set(command2, command1, command3);
#if SET_COMMAND_ACK
					uart_putc(RS_MOTOR);
#endif
					break;
				case CHAR_MOTOR_GET_SPEED:
				case CHAR_MOTOR_GET_DIRECTION:
					command1 = uart_getc(); // motor
					uart_putc(RS_MOTOR);
					uart_putc(motor_get(command2, command1));
					break;
				case CHAR_MOTOR_BRAKE:
					brake();
#if SET_COMMAND_ACK
					uart_putc(RS_MOTOR);
#endif
					break;
			};
			break;

		case RS_STOP_SERVER: // perform an emergency reset
			emergencyStopPerform();
			enabled = 0;
			uart_putc(RS_STOP_SERVER);

			lcd_clrscr();
			lcd_puts_P("INACTIVE\nsoftware stop");

			break;

		case RS_EXPANDER:
			command2 = uart_getc();
			if(command2 == RS_EXPANDER_SET)
			{
				i2c_exp_set_value(uart_getc());
#if SET_COMMAND_ACK
				uart_putc(RS_EXPANDER);
#endif
			}
			else if(command2 == RS_EXPANDER_GET)
			{
				uart_putc(RS_EXPANDER);
				uart_putc(i2c_exp_get_value());
			}
			break;

		case RS_ALL_DATA:
			if(uart_getc() != RS_ALL_DATA) break;

			uart_putc(RS_ALL_DATA);
			// battery
			uart_putc(RS_BATTERY);
			uart_putc((uint8_t)(0xff & voltage));
			uart_putc((uint8_t)(0xff & (voltage >> 8)));
			uart_putc((uint8_t)(0xff & current));
			uart_putc((uint8_t)(0xff & (current >> 8)));
			// arm directions
			uart_putc(RS_ARM);
			uart_putc(CHAR_ARM_GET_ALL_DIRECTIONS);
			uart_putc(arm_get(arm::MOTOR_SHOULDER, CHAR_ARM_GET_DIRECTION));
			uart_putc(arm_get(arm::MOTOR_ELBOW, CHAR_ARM_GET_DIRECTION));
			uart_putc(arm_get(arm::MOTOR_WRIST, CHAR_ARM_GET_DIRECTION));
			uart_putc(arm_get(arm::MOTOR_GRIPPER, CHAR_ARM_GET_DIRECTION));
			// arm positions
			uart_putc(RS_ARM);
			uart_putc(CHAR_ARM_GET_ALL_POSITIONS);
			uart_putc(arm_get(arm::MOTOR_SHOULDER, CHAR_ARM_GET_POSITION));
			uart_putc(arm_get(arm::MOTOR_ELBOW, CHAR_ARM_GET_POSITION));
			uart_putc(arm_get(arm::MOTOR_WRIST, CHAR_ARM_GET_POSITION));
			uart_putc(arm_get(arm::MOTOR_GRIPPER, CHAR_ARM_GET_POSITION));
			break;

		case RS_ARM:
			command2 = uart_getc(); // command for arm driver
			switch(command2)
			{
				case CHAR_ARM_SET_SPEED:
				case CHAR_ARM_SET_DIRECTION:
				case CHAR_ARM_SET_POSITION:
					command1 = uart_getc(); // which motor?
					command3 = uart_getc(); // value
					arm_set(command1, command3, command2);
#if SET_COMMAND_ACK
					uart_putc(RS_ARM);
#endif
					break;
				case CHAR_ARM_GET_SPEED:
				case CHAR_ARM_GET_DIRECTION:
				case CHAR_ARM_GET_POSITION:
					command3 = uart_getc(); // which motor?
					uart_putc(RS_ARM);
					uart_putc(arm_get(command3, command2));
					break;
				case CHAR_ARM_BRAKE:
					arm_brake();
#if SET_COMMAND_ACK
					uart_putc(RS_ARM);
#endif
					break;
				case CHAR_ARM_GET_ALL_POSITIONS:
					uart_putc(RS_ARM);
					uart_putc(CHAR_ARM_GET_ALL_POSITIONS);
					// order: shoulder, elbow, wrist, gripper
					uart_putc(arm_get(arm::MOTOR_SHOULDER, CHAR_ARM_GET_POSITION));
					uart_putc(arm_get(arm::MOTOR_ELBOW, CHAR_ARM_GET_POSITION));
					uart_putc(arm_get(arm::MOTOR_WRIST, CHAR_ARM_GET_POSITION));
					uart_putc(arm_get(arm::MOTOR_GRIPPER, CHAR_ARM_GET_POSITION));
					break;
				case CHAR_ARM_GET_ALL_DIRECTIONS:
					uart_putc(RS_ARM);
					uart_putc(CHAR_ARM_GET_ALL_DIRECTIONS);
					// order: shoulder, elbow, wrist, gripper
					uart_putc(arm_get(arm::MOTOR_SHOULDER, CHAR_ARM_GET_DIRECTION));
					uart_putc(arm_get(arm::MOTOR_ELBOW, CHAR_ARM_GET_DIRECTION));
					uart_putc(arm_get(arm::MOTOR_WRIST, CHAR_ARM_GET_DIRECTION));
					uart_putc(arm_get(arm::MOTOR_GRIPPER, CHAR_ARM_GET_DIRECTION));
					break;
				case CHAR_ARM_GET_MODE:
				case CHAR_ARM_IS_CALIBRATED:
					uart_putc(RS_ARM);
					uart_putc(arm_get_one_byte(command2));
					break;
				case CHAR_ARM_CALIBRATE:
					arm_calibrate();
#if SET_COMMAND_ACK
					uart_putc(RS_ARM);
#endif
					break;
			};
			break;
	}
	else
	{
		if(command1 == RS_INITIALIZATION)
		{
			emergencyStopPerform();
			_delay_us(4);
			emergencyStopDisable();
			enabled = 1;

			menuSetToMain();

			lcd_clrscr();
			lcd_puts_P("OPERATIONAL");

			uart_putc(RS_INITIALIZATION);
		}
	}
}

// activate bus reset as output and keep low
void emergencyStopPerform()
{
	DDR(EMERGENCY_STOP_PORT) |= (1<<EMERGENCY_STOP);
	PORT(EMERGENCY_STOP_PORT) &= ~(1<<EMERGENCY_STOP);
	i2c_exp_set_value(0);
}
// set as input, disable pullup
void emergencyStopDisable()
{
	DDR(EMERGENCY_STOP_PORT) &= ~(1<<EMERGENCY_STOP);
	PORT(EMERGENCY_STOP_PORT) &= ~(1<<EMERGENCY_STOP);
}

uint8_t emergencyIsStopped()
{
	return !(PIN(EMERGENCY_STOP_PORT) & (1<<EMERGENCY_STOP));
}

