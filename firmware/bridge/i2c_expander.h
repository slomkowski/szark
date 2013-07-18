/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _I2C_EXPANDER_H_
#define _I2C_EXPANDER_H_

#define I2C_EXPANDER_ADDRESS 0x64

void i2c_exp_set_value(uint8_t data);
uint8_t i2c_exp_get_value();

#endif
