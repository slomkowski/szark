/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "i2c.h"
#include "arm.h"

uint8_t arm_get(uint8_t arm, uint8_t command)
{
	uint8_t val;

	i2c_start();
	
	i2c_write((ARM_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(command);
	i2c_write(arm);
	
	i2c_start();
	i2c_write((ARM_DRIVER_ADDRESS << 1)| I2C_READ_MASK);

	val = i2c_read(I2C_NACK);
	
	i2c_stop();

	return val;
}

void arm_set(uint8_t arm, uint8_t value, uint8_t command)
{
	i2c_start();

	i2c_write((ARM_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(command);
	i2c_write(arm);
	i2c_write(value);

	i2c_stop();
}

void arm_one_byte(uint8_t byte)
{
	i2c_start();

	i2c_write((ARM_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(byte);

	i2c_stop();	
}

uint8_t arm_get_one_byte(uint8_t command)
{
	uint8_t val;

	i2c_start();
	
	i2c_write((ARM_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(command);
	
	i2c_start();
	i2c_write((ARM_DRIVER_ADDRESS << 1)| I2C_READ_MASK);

	val = i2c_read(I2C_NACK);
	
	i2c_stop();
	return val;
}

bool arm_is_calibrated()
{
	if(arm_get_one_byte(CHAR_ARM_IS_CALIBRATED) == CHAR_ARM_TRUE) return true;
	else return false;
}
