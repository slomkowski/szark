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
#include "usb_support.hpp"
#include "usb_processcommands.hpp"
#include "led.hpp"

//
static const bool WATCHDOG_ENABLE = true;

static void checkButtonsResetCombination() {
	auto btn = buttons::getButtonsState(false);

	// reset the device when special combination happens
	if (btn->down and btn->enter) {
		lcd::clrscr();
		lcd_putsP("FIRMWARE UPDATE");

		wdt_enable(WDTO_15MS);
		while (true) {
		}
	}
}

int main(void) {
	if (WATCHDOG_ENABLE) {
		wdt_enable(WDTO_1S);
	}

	killswitch::init();
	led::init();
	i2c::init();
	lcd::init();
	buttons::init();
	menu::init();
	usb::init();

	lcd_putsP("SZARK - Loading\n2014 Slomkowski");

	_delay_ms(250);

	sei();

	analog::init();

	for (;;) {
		if (WATCHDOG_ENABLE) {
			wdt_reset();
		}

		if (usb::isMenuEnabled()) {
			menu::poll();
			delay::waitMs(100);
		}

		usb::poll();

		checkButtonsResetCombination();

		killswitch::checkHardware();
	}
}
