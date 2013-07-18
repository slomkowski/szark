/*
 #  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
 #  module bridge - converter between RS232 and I2C main bus
 #  Michał Słomkowski 2011, 2012
 #  www.slomkowski.eu m.slomkowski@gmail.com
 */

#include "global.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "lcd.h"

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
#define LCDC_SHIFTR		0x04
#define LCDC_SHIFTL		0
#define LCDC_FUNC		0x20
#define LCDC_FUNC8b		0x10
#define LCDC_FUNC4b		0
#define LCDC_FUNC2L		0x08
#define LCDC_FUNC1L		0
#define LCDC_FUNC5x10	0x4
#define LCDC_FUNC5x7	0
#define LCDC_CGA		0x40
#define LCDC_DDA		0x80

#define E_HIGH() { PORT(LCD_E_PORT) |= (1 << LCD_E); }
#define RS_HIGH() { PORT(LCD_RS_PORT) |= (1 << LCD_RS); }

#define D4_HIGH() { PORT(LCD_D4_PORT) |= (1 << LCD_D4); }
#define D5_HIGH() { PORT(LCD_D5_PORT) |= (1 << LCD_D5); }
#define D6_HIGH() { PORT(LCD_D6_PORT) |= (1 << LCD_D6); }
#define D7_HIGH() { PORT(LCD_D7_PORT) |= (1 << LCD_D7); }

#define E_LOW() { PORT(LCD_E_PORT) &= ~(1 << LCD_E); }
#define RS_LOW() { PORT(LCD_RS_PORT) &= ~(1 << LCD_RS); }

#define D4_LOW() { PORT(LCD_D4_PORT) &= ~(1 << LCD_D4); }
#define D5_LOW() { PORT(LCD_D5_PORT) &= ~(1 << LCD_D5); }
#define D6_LOW() { PORT(LCD_D6_PORT) &= ~(1 << LCD_D6); }
#define D7_LOW() { PORT(LCD_D7_PORT) &= ~(1 << LCD_D7); }

// 2 instrukcje trwają 250ns
#define delay250ns() { asm volatile("nop\n\tnop"::); }

// making pulse on Enable line
#define E_PULSE() { E_HIGH(); delay250ns(); delay250ns(); E_LOW(); }

#define lcd_command(data) { RS_LOW(); lcd_send(data); }

// funkcje opóźnień na bazie kursu C z EdW 10/2005

// funkcja opóźninia 1us : DEC - 1 cykl, BRNE 2 cykle, + 5xnop. Zegar 8MHz
#define delayus8(t)\
	{asm volatile( \
		"delayus8_loop%=: \n\t"\
			"nop \n\t"\
			"nop \n\t"\
			"nop \n\t"\
			"nop \n\t"\
			"nop \n\t"\
			"dec %[ticks] \n\t"\
			"brne delayus8_loop%= \n\t"\
	: :[ticks]"r"(t) );}

// variable which stores the position of the cursor
static uint8_t pos;

static void lcd_send(uint8_t data) {
	D4_LOW()
	;
	D5_LOW()
	;
	D6_LOW()
	;
	D7_LOW()
	;
	// najpierw starsza część bajtu
	if (data & 0x80) D7_HIGH()
	;
	if (data & 0x40) D6_HIGH()
	;
	if (data & 0x20) D5_HIGH()
	;
	if (data & 0x10) D4_HIGH()
	;

	E_PULSE()
	;

	D4_LOW()
	;
	D5_LOW()
	;
	D6_LOW()
	;
	D7_LOW()
	;
	// teraz młodsza
	if (data & 0x08) D7_HIGH()
	;
	if (data & 0x04) D6_HIGH()
	;
	if (data & 0x02) D5_HIGH()
	;
	if (data & 0x01) D4_HIGH()
	;

	E_PULSE()
	;

	/*D7_HIGH();
	 D6_HIGH();
	 D5_HIGH();
	 D4_HIGH();*/

	delayus8(40); // datasheet says it should be at least 37us
}

static void lcd_data(uint8_t data) {
	RS_HIGH()
	;
	lcd_send(data);
}

void lcd_putc(uint8_t data) {
	if ((data == '\n')) {
		lcd_command(LCDC_DDA|64);
		pos = 0;
	} else {
		if (pos == LCD_COLUMNS) {
			lcd_command(LCDC_DDA|64);
			pos = 0;
		}
		lcd_data(data);
		pos++;
	}
}

void lcd_init() {
	// setting D4-D7, RS and E as outputs
	DDR(LCD_E_PORT) |= (1 << LCD_E);
	DDR(LCD_RS_PORT) |= (1 << LCD_RS);

	DDR(LCD_D4_PORT) |= (1 << LCD_D4);
	DDR(LCD_D5_PORT) |= (1 << LCD_D5);
	DDR(LCD_D6_PORT) |= (1 << LCD_D6);
	DDR(LCD_D7_PORT) |= (1 << LCD_D7);

	_delay_ms(15); // to make the supply voltage stable

	RS_LOW()
	;

	D4_HIGH()
	;
	D5_HIGH()
	;
	D6_LOW()
	;
	D7_LOW()
	;

	E_PULSE()
	;
	_delay_ms(5); // more than 4.1ms

	E_PULSE()
	;
	delayus8(150); // more than 100us

	D4_LOW()
	;
	E_PULSE()
	;
	delayus8(150);

	// już jest 4 bit tryb, już można pisać normalnie

	lcd_send(LCDC_FUNC | LCDC_FUNC4b | LCDC_FUNC2L | LCDC_FUNC5x7);
	lcd_send(LCDC_MODE | LCDC_MODER);
	lcd_send(LCDC_ON | LCDC_ONDISPLAY);
	//lcd_send(LCDC_ON);
	lcd_send(LCDC_CLS);
	_delay_ms(2);

	//RS_HIGH();

	pos = 0;
}

void lcd_puts(char *s) {
	char c;
	while ((c = *s++))
		lcd_putc(c);
}

void lcd_puts_p(const char *progmem_s) {
	char c;
	while ((c = pgm_read_byte(progmem_s++)))
		lcd_putc(c);
}

void lcd_clrscr() {
	pos = 0;

	lcd_command(LCDC_CLS);

	_delay_ms(2);
}

