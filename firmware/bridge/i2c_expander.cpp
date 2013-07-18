/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#include "i2c.h"
#include "i2c_expander.h"

#define WRITE_AND_TRANSFER_DATA 0x44
#define WRITE_DATA 0x11
#define TRANSFER_DATA 0x22
#define READ_DATA 0x11

void i2c_exp_set_value(uint8_t data)
{
	i2c_start();

	i2c_write((I2C_EXPANDER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(WRITE_AND_TRANSFER_DATA);
	i2c_write(data);

	i2c_stop();
}

uint8_t i2c_exp_get_value()
{
	uint8_t val;

	i2c_start();

	i2c_write((I2C_EXPANDER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(READ_DATA);

	i2c_start();
	i2c_write((I2C_EXPANDER_ADDRESS << 1) | I2C_READ_MASK);
	val = i2c_read(I2C_NACK);

	i2c_stop();

	return val;
}

