/*
#    Arm driver - SZARK
#  Michał Słomkowski 2011, 2012
#  www.flylab.ovh.org m.slomkowski@gmail.com
*/

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "joint.h"
#include "i2c-slave.h"

using namespace arm;

void processCommands(uint8_t *rxbuf, uint8_t *txbuf);

int main()
{
	joint::init();

	// i2c
	TWI_Slave_Initialise((unsigned char)((I2C_SLAVE_ADDRESS << TWI_ADR_BITS) | (1<<TWI_GEN_BIT)), processCommands);
	TWI_Start_Transceiver();

#if WATCHD0G_ENABLE
	wdt_enable(WDTO_120MS);
#endif

	sei();

	while(TRUE)
	{
		if(joint::startCalibration) joint::calibrate();

#if WATCHD0G_ENABLE
		wdt_enable(WDTO_120MS);
#endif
	}
}

// this function is called within interrupt routine, should be as fast as possible
void processCommands(uint8_t *rxbuf, uint8_t *txbuf)
{
	// command interpreter
	switch(rxbuf[0])
	{
		case CHAR_ARM_GET_SPEED:
			txbuf[0] = joint::getSpeed((Motor)rxbuf[1]);
			break;
		case CHAR_ARM_SET_SPEED:
			joint::interruptCalibration = true;
			joint::setSpeed((Motor)rxbuf[1], rxbuf[2]);
			break;
		case CHAR_ARM_SET_DIRECTION:
			joint::interruptCalibration = true;
			joint::setDirection((Motor)rxbuf[1], (Direction)rxbuf[2]);
			break;
		case CHAR_ARM_GET_DIRECTION:
			txbuf[0] = joint::getDirection((Motor)rxbuf[1]);
			break;
		case CHAR_ARM_GET_POSITION:
			txbuf[0] = joint::getPosition((Motor)rxbuf[1]);
			break;
		case CHAR_ARM_GET_MODE:
			txbuf[0] = joint::getMode((Motor)rxbuf[1]);
			break;
		case CHAR_ARM_IS_CALIBRATED:
			txbuf[0] = joint::calibrated ? CHAR_ARM_TRUE : CHAR_ARM_FALSE;
			break;
		case CHAR_ARM_SET_POSITION:
			joint::interruptCalibration = true;
			joint::setPosition((Motor)rxbuf[1], rxbuf[2]);
			break;
		case CHAR_ARM_BRAKE:
			joint::interruptCalibration = true;
			joint::brake();
			break;
		case CHAR_ARM_CALIBRATE:
			joint::interruptCalibration = true;
			joint::startCalibration = true;
			break;
	};
}

