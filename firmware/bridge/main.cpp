#include "global.hpp"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "i2c.hpp"
#include "lcd.hpp"
#include "analog.hpp"
#include "buttons.hpp"
#include "menu.hpp"
#include "delay.hpp"
#include "killswitch.hpp"
#include "usb_request.hpp"

//
static const bool WATCHDOG_ENABLE = false;

int main(void) {
	if (WATCHDOG_ENABLE) {
		wdt_enable(WDTO_1S);
	}

	killswitch::init();

	i2c::init();
	lcd::init();
	buttons::init();
	menu::init();

	lcd_putsP("SZARK - Loading\n2014 Slomkowski");

	_delay_ms(250);

	sei();
	analog::init();

	for (;;) {
		if (WATCHDOG_ENABLE) {
			wdt_reset();
		}

		usb::executeCommandsFromUSB();

		if (not usb::wasKillSwitchDisabled()) {
			menu::poll();
			delay::waitMs(150);
		}
	}
}
