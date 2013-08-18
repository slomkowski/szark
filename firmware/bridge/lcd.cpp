/*
 * lcd.cpp
 *
 *  Created on: 06-08-2013
 *      Author: michal
 */

#include "global.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "delay.h"
#include "lcd.h"

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

#define LCDC_CLS		0x01
#define LCDC_HOME		0x02
#define LCDC_MODE		0x04
#define LCDC_MODER		0x02
#define LCDC_MODEL		0
#define LCDC_MODEMOVE	0x01
#define LCDC_ON 		0x08
#define LCDC_ONDISPLAY	0x04
#define LCDC_ONCURSOR	0x02
#define LCDC_ONBLINK	0x01
#define LCDC_SHIFT		0x10
#define LCDC_SHIFTDISP	0x08
#define LCDC_SHIFTR	0x04
#define LCDC_SHIFTL	0
#define LCDC_FUNC		0x20
#define LCDC_FUNC8b	0x10
#define LCDC_FUNC4b	0
#define LCDC_FUNC2L	0x08
#define LCDC_FUNC1L	0
#define LCDC_FUNC5x10	0x4
#define LCDC_FUNC5x7	0
#define LCDC_CGA		0x40
#define LCDC_DDA		0x80
#define LCDC_LINE1  	0x80
#define LCDC_LINE2  	0xC0

enum Type {
	DATA, COMMAND
};

using namespace lcd;

static uint8_t position;

static void pulseE() {
	PORT(LCD_E_PORT) |= (1 << LCD_E);
	delay::wait100us();
	delay::wait100us();
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);
}

static void updateBus(uint8_t data) {
	PORT(LCD_D4_PORT) &= ~(1 << LCD_D4);
	PORT(LCD_D5_PORT) &= ~(1 << LCD_D5);
	PORT(LCD_D6_PORT) &= ~(1 << LCD_D6);
	PORT(LCD_D7_PORT) &= ~(1 << LCD_D7);

	if (data & 0x80) PORT(LCD_D7_PORT) |= (1 << LCD_D7);
	if (data & 0x40) PORT(LCD_D6_PORT) |= (1 << LCD_D6);
	if (data & 0x20) PORT(LCD_D5_PORT) |= (1 << LCD_D5);
	if (data & 0x10) PORT(LCD_D4_PORT) |= (1 << LCD_D4);
}

static void send(uint8_t data, Type type) {

	if (type == COMMAND) {
		PORT(LCD_RS_PORT) &= ~(1 << LCD_RS);
	} else {
		PORT(LCD_RS_PORT) |= (1 << LCD_RS);
	}
	updateBus(data);
	pulseE();
	updateBus(data << 4);
	pulseE();

	// Delay 636 cycles
	// 39us 750 ns at 16 MHz

	asm volatile (
		"    ldi  r18, 212" "\n"
		"1:  dec  r18" "\n"
		"    brne 1b" "\n"
	);
}

void lcd::init() {
	// setting D4-D7, RS and E as outputs
	DDR(LCD_E_PORT) |= (1 << LCD_E);
	DDR(LCD_RS_PORT) |= (1 << LCD_RS);

	DDR(LCD_D4_PORT) |= (1 << LCD_D4);
	DDR(LCD_D5_PORT) |= (1 << LCD_D5);
	DDR(LCD_D6_PORT) |= (1 << LCD_D6);
	DDR(LCD_D7_PORT) |= (1 << LCD_D7);

	PORT(LCD_RS_PORT) |= (1 << LCD_RS);
	PORT(LCD_E_PORT) |= (1 << LCD_E);

	_delay_ms(25);

	PORT(LCD_RS_PORT) &= ~(1 << LCD_RS);
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);

	PORT(LCD_D4_PORT) |= (1 << LCD_D4);
	PORT(LCD_D5_PORT) |= (1 << LCD_D5);
	PORT(LCD_D6_PORT) &= ~(1 << LCD_D6);
	PORT(LCD_D7_PORT) &= ~(1 << LCD_D7);

	pulseE();
	_delay_ms(5); // more than 4.1ms

	pulseE();
	delay::wait100us();

	pulseE();
	delay::wait100us();

	PORT(LCD_D4_PORT) &= ~(1 << LCD_D4);
	pulseE();
	delay::wait100us();

	send(LCDC_FUNC | LCDC_FUNC4b | LCDC_FUNC2L | LCDC_FUNC5x7, COMMAND);
	send(LCDC_ON | 0, COMMAND);

	clrscr();

	send(LCDC_MODE | LCDC_MODER, COMMAND);
	send(LCDC_ON | LCDC_ONDISPLAY, COMMAND);
}

void lcd::clrscr() {
	gotoxy(0, 0);

	send(LCDC_CLS, COMMAND);

	delay::waitMs(2);
}

void lcd::putc(char character) {
	if ((character == '\n')) {
		send(LCDC_LINE2 | 64, COMMAND);
		position = 0;
	} else {
		if (position == LCD_COLUMNS) {
			send(LCDC_LINE2 | 64, COMMAND);
			position = 0;
		}
		send(character, DATA);
		position++;
	}
}

void lcd::gotoxy(uint8_t x, uint8_t y) {
	if (y == 0) {
		send(LCDC_LINE1 | x, COMMAND);
	} else {
		send(LCDC_LINE2 | x, COMMAND);
	}
	position = x;
}

void lcd::puts(const char *str, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		char c = static_cast<char>(str[i]);
		if (c == '\0') {
			break;
		}
		putc(c);
	}
}

void lcd::puts(uint8_t x, uint8_t y, const char *str) {
	gotoxy(x, y);
	puts(str);
}

void lcd::putsp(const char *progmemString) {
	char c;
	while ((c = pgm_read_byte(progmemString++))) {
		putc(c);
	}
}

void lcd::putsp(uint8_t x, uint8_t y, const char *progmem_s) {
	gotoxy(x, y);
	putsp(progmem_s);
}

