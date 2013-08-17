/*
 * lcd.h
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#ifndef LCD_H_
#define LCD_H_

namespace lcd {
	const uint8_t LCD_COLUMNS = 16;
	const uint8_t LCD_ROWS = 2;

	void init();
	void clrscr();

	void putc(char character);

	void gotoxy(uint8_t x, uint8_t y);

	void puts(const char *str, uint8_t length = LCD_COLUMNS * LCD_ROWS + 1);
	void puts(uint8_t x, uint8_t y, const char *str);

	void putsp(const char *progmem_s);
	void putsp(uint8_t x, uint8_t y, const char *progmemString);
}

#include <avr/pgmspace.h>
#define lcd_putsP(text) lcd::putsp(PSTR(text))

#endif /* LCD_H_ */
