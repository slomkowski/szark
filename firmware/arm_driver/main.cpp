/*
#    Arm driver - SZARK
#  Michał Słomkowski 2011, 2012
#  www.flylab.ovh.org m.slomkowski@gmail.com
*/

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "motors.h"
#include "i2c-slave.h"

void process_commands(uint8_t *rxbuf, uint8_t *txbuf);

int main()
{
	motor_init();

	// i2c
	TWI_Slave_Initialise((unsigned char)((I2C_SLAVE_ADDRESS << TWI_ADR_BITS) | (1<<TWI_GEN_BIT)), process_commands);
	TWI_Start_Transceiver();

#if WATCHD0G_ENABLE
	wdt_enable(WDTO_120MS);
#endif

	sei();

	while(TRUE)
	{
		if(startCalibration) motor_calibrate();

#if WATCHD0G_ENABLE
		wdt_enable(WDTO_120MS);
#endif
	}
}

// this function is called within interrupt routine, should be as fast as possible
void process_commands(uint8_t *rxbuf, uint8_t *txbuf)
{
	// command interpreter
	switch(rxbuf[0])
	{
		case CHAR_ARM_GET_SPEED:
			txbuf[0] = motor_get_speed((MOTOR)rxbuf[1]);
			break;
		case CHAR_ARM_SET_SPEED:
			interruptCalibration = true;
			motor_set_speed((MOTOR)rxbuf[1], rxbuf[2]);
			break;
		case CHAR_ARM_SET_DIRECTION:
			interruptCalibration = true;
			motor_set_direction((MOTOR)rxbuf[1], (DIRECTION)rxbuf[2]);
			break;
		case CHAR_ARM_GET_DIRECTION:
			txbuf[0] = motor_get_direction((MOTOR)rxbuf[1]);
			break;
		case CHAR_ARM_GET_POSITION:
			txbuf[0] = motor_get_position((MOTOR)rxbuf[1]);
			break;
		case CHAR_ARM_GET_MODE:
			txbuf[0] = motor_get_mode((MOTOR)rxbuf[1]);
			break;
		case CHAR_ARM_IS_CALIBRATED:
			txbuf[0] = calibrated ? CHAR_ARM_TRUE : CHAR_ARM_FALSE;
			break;
		case CHAR_ARM_SET_POSITION:
			interruptCalibration = true;
			motor_set_position((MOTOR)rxbuf[1], rxbuf[2]);
			break;
		case CHAR_ARM_BRAKE:
			interruptCalibration = true;
			motor_brake();
			break;
		case CHAR_ARM_CALIBRATE:
			interruptCalibration = true;
			startCalibration = true;
			break;
	};
}

