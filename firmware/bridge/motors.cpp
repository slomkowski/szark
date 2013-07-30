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
#include "motors.h"

uint8_t motor_get(uint8_t command, uint8_t motor)
{
	uint8_t val;

	i2c_start();
	
	i2c_write((MOTOR_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(command);
	i2c_write(motor);
	
	i2c_start();
	i2c_write((MOTOR_DRIVER_ADDRESS << 1)| I2C_READ_MASK);

	val = i2c_read(I2C_NACK);
	
	i2c_stop();

	return val;
}

void motor_set(uint8_t command, uint8_t motor, uint8_t value)
{
	i2c_start();

	i2c_write((MOTOR_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(command);
	i2c_write(motor);
	i2c_write(value);

	i2c_stop();
}

void brake()
{
	i2c_start();

	i2c_write((MOTOR_DRIVER_ADDRESS << 1) | I2C_WRITE_MASK);
	i2c_write(CHAR_MOTOR_BRAKE);

	i2c_stop();	
}

