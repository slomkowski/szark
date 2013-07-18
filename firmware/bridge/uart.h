/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

/* support for hardware uart */

#ifndef _UART_H_
#define _UART_H_

// UART speed in bps
#define UART_BAUDRATE 38400UL

#define uart_enable_interrupt() UCSRB |= (1 << RXCIE)
#define uart_disable_interrupt() UCSRB &= ~(1 << RXCIE)

void uart_init();
int uart_putc(char byte);
int uart_getc(void);
void uart_puts(char *data);
void uart_puts_p(const char *data);

#endif

