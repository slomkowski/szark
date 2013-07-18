/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

/* support for hardware TWI (I2C) interface */

#ifndef _I2C_H_
#define _I2C_H_

#include "global.h"

#include <inttypes.h>

/* I2C clock frequency in kHz */
#define I2C_CLOCK  50

#define I2C_ACK 1
#define I2C_NACK 0

#define I2C_WRITE_MASK	0x0
#define I2C_READ_MASK	0x1

void i2c_init();

void i2c_start();
void i2c_stop();

void i2c_write(uint8_t byte);
uint8_t i2c_read(uint8_t ack);

//uint8_t i2c_select_read(char ucDevice);

//uint8_t i2c_select_write(char ucDevice);
//uint8_t i2c_write(char* ucData, char ucLen);
//uint8_t i2c_read(char* ucData, char ucLen);

#endif
