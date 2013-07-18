/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

/* support for hardware uart */

#include "global.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "uart.h"

#define UART_PARITY_NONE 0 
#define UART_PARITY_EVEN (1<<UPM1) 
#define UART_PARITY_ODD ((1<<UPM1)|(1<<UPM0)) 
#define UART_STOP_BITS_1 0 
#define UART_STOP_BITS_2 (1<<USBS) 
#define UART_DATA_BITS_5 0 
#define UART_DATA_BITS_6 (1<<UCSZ0) 
#define UART_DATA_BITS_7 (1<<UCSZ1) 
#define UART_DATA_BITS_8 ((1<<UCSZ1)|(1<<UCSZ0)) 
#define UART_DATA_BITS_9 ((1<<UCSZ2)|(1<<UCSZ1)|(1<<UCSZ0)) 

#define UART_UBBR_ ((F_CPU/(16*UART_BAUDRATE))-1)

void uart_init() 
{ 
	UBRRH = (unsigned char)(UART_UBBR_>>8); 
	UBRRL = (unsigned char)UART_UBBR_; 
	UCSRB = (1 << RXEN) | (1 << TXEN); 
	UCSRC = (1 << URSEL) | UART_DATA_BITS_8 | UART_PARITY_NONE | UART_STOP_BITS_1; 
	UCSRA = 0;
}

int uart_putc(char byte)
{
	while(!(1<<UDRE & UCSRA));
	UDR = byte;
	return 0;
}

int uart_getc(void)
{
	while (!(1<<RXC & UCSRA));
	return UDR;
}

void uart_puts(char *data)
{
	while(*data != '\0') uart_putc(*data++);
}

void uart_puts_p(const char *data)
{
	while(pgm_read_byte(*data) != '\0') uart_putc(pgm_read_byte(*data++));
}

