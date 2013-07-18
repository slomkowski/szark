/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef LCD_H
#define LCD_H

#include "global.h"

// configuration of the LCD pinout

#define LCD_D4_PORT	D
#define LCD_D4		7

#define LCD_D5_PORT	B
#define LCD_D5		0

#define LCD_D6_PORT	B
#define LCD_D6		1

#define LCD_D7_PORT	B
#define LCD_D7		2

#define LCD_E_PORT	D
#define LCD_E		6

#define LCD_RS_PORT D
#define LCD_RS		5

#define LCD_COLUMNS 16
#define LCD_ROWS 2

#include <inttypes.h>

void lcd_init();
void lcd_putc(uint8_t c);
void lcd_puts(char *s);
void lcd_puts_p(const char *progmem_s);

void lcd_clrscr();

#include <avr/pgmspace.h>

#define lcd_puts_P(__s)         lcd_puts_p(PSTR(__s))

#endif //LCD_H
