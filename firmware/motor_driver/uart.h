/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja
#  moduł bridge - most między RS232 i magistralą główną
#  Michał Słomkowski 2011
#  www.flylab.ovh.org m.slomkowski@gmail.com
*/

/* support for hardware uart */

#ifndef _UART_H_
#define _UART_H_

// UART speed in bps
#define UART_BAUDRATE 19200UL

#define uart_enable_interrupt() UCSRB |= (1 << RXCIE)
#define uart_disable_interrupt() UCSRB &= ~(1 << RXCIE)

inline void uart_init();

int uart_putc(char byte);
int uart_getc(void);
void uart_puts(char *data);
void uart_puts_p(const char *data);

#endif

