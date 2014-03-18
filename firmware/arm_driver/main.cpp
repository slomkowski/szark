/*
 #    Arm driver - SZARK
 #  Michał Słomkowski 2011, 2012
 #  www.flylab.ovh.org m.slomkowski@gmail.com
 */

#include "global.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "joint.hpp"
#include "i2c-slave.hpp"
#include "arm_driver-commands.hpp"

void processCommands(uint8_t *rxbuf, uint8_t *txbuf);

int main() {
	joint::init();

	// i2c
	TWI_Slave_Initialise((unsigned char) ((I2C_SLAVE_ADDRESS << TWI_ADR_BITS) | (1 << TWI_GEN_BIT)), processCommands);
	TWI_Start_Transceiver();

#if WATCHD0G_ENABLE
	wdt_enable(WDTO_120MS);
#endif

	sei();

	while (true) {
		if (joint::startCalibration) joint::calibrate();

#if WATCHD0G_ENABLE
		wdt_reset();
#endif
	}
}

// this function is called within interrupt routine, should be as fast as possible
void processCommands(uint8_t *rxbuf, uint8_t *txbuf) {
	// command interpreter
	switch (rxbuf[0]) {
	case arm::GET_SPEED:
		txbuf[0] = joint::getSpeed((arm::Motor) rxbuf[1]);
		break;
	case arm::SET_SPEED:
		joint::interruptCalibration = true;
		joint::setSpeed((arm::Motor) rxbuf[1], rxbuf[2]);
		break;
	case arm::SET_DIRECTION:
		joint::interruptCalibration = true;
		joint::setDirection((arm::Motor) rxbuf[1], (arm::Direction) rxbuf[2]);
		break;
	case arm::GET_DIRECTION:
		txbuf[0] = joint::getDirection((arm::Motor) rxbuf[1]);
		break;
	case arm::GET_POSITION:
		txbuf[0] = joint::getPosition((arm::Motor) rxbuf[1]);
		break;
	case arm::GET_MODE:
		txbuf[0] = joint::getMode((arm::Motor) rxbuf[1]);
		break;
	case arm::IS_CALIBRATED:
		txbuf[0] = joint::calibrated ? arm::TRUE : arm::FALSE;
		break;
	case arm::SET_POSITION:
		joint::interruptCalibration = true;
		joint::setPosition((arm::Motor) rxbuf[1], rxbuf[2]);
		break;
	case arm::BRAKE:
		joint::interruptCalibration = true;
		joint::brake();
		break;
	case arm::CALIBRATE:
		joint::interruptCalibration = true;
		joint::startCalibration = true;
		break;
	};
}

