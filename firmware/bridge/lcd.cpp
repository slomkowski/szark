/*
 * lcd.cpp
 *
 *  Project: bridge
 *  Created on: 24-08-2013
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include "global.hpp"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "delay.hpp"
#include "lcd.hpp"

#define LCD_D4_PORT	B
#define LCD_D4			3

#define LCD_D5_PORT	B
#define LCD_D5			2

#define LCD_D6_PORT	B
#define LCD_D6			6

#define LCD_D7_PORT	B
#define LCD_D7			5

#define LCD_E_PORT		B
#define LCD_E			1

#define LCD_RS_PORT	F
#define LCD_RS			6

#define LCD_RW_PORT	F
#define LCD_RW			7

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

enum class Type {
	DATA, COMMAND
};

enum class LcdDir {
	IN, OUT
};

using namespace lcd;

static uint8_t position;

static void setDirection(LcdDir direction) {
	if (direction == LcdDir::OUT) {
		DDR(LCD_D4_PORT) |= (1 << LCD_D4);
		DDR(LCD_D5_PORT) |= (1 << LCD_D5);
		DDR(LCD_D6_PORT) |= (1 << LCD_D6);
		DDR(LCD_D7_PORT) |= (1 << LCD_D7);
	} else {
		DDR(LCD_D4_PORT) &= ~(1 << LCD_D4);
		DDR(LCD_D5_PORT) &= ~(1 << LCD_D5);
		DDR(LCD_D6_PORT) &= ~(1 << LCD_D6);
		DDR(LCD_D7_PORT) &= ~(1 << LCD_D7);

		PORT(LCD_D4_PORT) &= ~(1 << LCD_D4);
		PORT(LCD_D5_PORT) &= ~(1 << LCD_D5);
		PORT(LCD_D6_PORT) &= ~(1 << LCD_D6);
		PORT(LCD_D7_PORT) &= ~(1 << LCD_D7);
	}
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

static uint8_t readBus() {
	uint8_t output = 0;

	if (PIN(LCD_D7_PORT) & (1 << LCD_D7)) {
		output |= 0x80;
	}

	if (PIN(LCD_D6_PORT) & (1 << LCD_D6)) {
		output |= 0x40;
	}

	if (PIN(LCD_D5_PORT) & (1 << LCD_D5)) {
		output |= 0x20;
	}

	if (PIN(LCD_D4_PORT) & (1 << LCD_D4)) {
		output |= 0x10;
	}

	return output;
}

static uint8_t receive() {

	uint8_t output = 0;

	setDirection(LcdDir::IN);

	PORT(LCD_RW_PORT) |= (1 << LCD_RW);

	PORT(LCD_E_PORT) |= (1 << LCD_E);
	output |= readBus();
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);

	PORT(LCD_E_PORT) |= (1 << LCD_E);
	output |= readBus() >> 4;
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);

	return output;
}

static uint8_t receiveStatus() {
	PORT(LCD_RS_PORT) &= ~(1 << LCD_RS);
	return receive();
}

static void send(uint8_t data, Type type) {
	setDirection(LcdDir::OUT);

	if (type == Type::COMMAND) {
		PORT(LCD_RS_PORT) &= ~(1 << LCD_RS);
	} else {
		PORT(LCD_RS_PORT) |= (1 << LCD_RS);
	}

	PORT(LCD_RW_PORT) &= ~(1 << LCD_RW);

	PORT(LCD_E_PORT) |= (1 << LCD_E);
	updateBus(data);
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);

	PORT(LCD_E_PORT) |= (1 << LCD_E);
	updateBus(data << 4);
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);

	while (receiveStatus() & 0x80) {
	}
}

static void pulseE() {
	PORT(LCD_E_PORT) |= (1 << LCD_E);
	delay::wait100us();
	delay::wait100us();
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);
}

void lcd::init() {
	// setting D4-D7, RS, RW and E as outputs
	DDR(LCD_E_PORT) |= (1 << LCD_E);
	DDR(LCD_RS_PORT) |= (1 << LCD_RS);
	DDR(LCD_RW_PORT) |= (1 << LCD_RW);

	setDirection(LcdDir::OUT);

	PORT(LCD_RS_PORT) |= (1 << LCD_RS);
	PORT(LCD_E_PORT) |= (1 << LCD_E);

	_delay_ms(25);

	PORT(LCD_RS_PORT) &= ~(1 << LCD_RS);
	PORT(LCD_E_PORT) &= ~(1 << LCD_E);
	PORT(LCD_RW_PORT) &= ~(1 << LCD_RW);

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

	send(LCDC_FUNC | LCDC_FUNC4b | LCDC_FUNC2L | LCDC_FUNC5x7, Type::COMMAND);
	send(LCDC_ON | 0, Type::COMMAND);

	clrscr();

	send(LCDC_MODE | LCDC_MODER, Type::COMMAND);
	send(LCDC_ON | LCDC_ONDISPLAY, Type::COMMAND);
}

void lcd::clrscr() {
	gotoxy(0, 0);

	send(LCDC_CLS, Type::COMMAND);
}

void lcd::putc(char character) {
	if ((character == '\n')) {
		send(LCDC_LINE2 | 64, Type::COMMAND);
		position = 0;
	} else {
		if (position == LCD_COLUMNS) {
			send(LCDC_LINE2 | 64, Type::COMMAND);
			position = 0;
		}
		send(character, Type::DATA);
		position++;
	}
}

void lcd::gotoxy(uint8_t x, uint8_t y) {
	if (y == 0) {
		send(LCDC_LINE1 | x, Type::COMMAND);
	} else {
		send(LCDC_LINE2 | x, Type::COMMAND);
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

