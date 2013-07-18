/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#include "global.h"

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#include "buttons.h"

uint8_t buttonPressed(uint8_t button)
{
        if (bit_is_clear(PIN(BUTTON_PORT), button))
        {
                _delay_ms(BUTTON_DEBOUNCE_TIME);
                if (bit_is_clear(PIN(BUTTON_PORT), button)) return 1;
        }

        return 0;
}

