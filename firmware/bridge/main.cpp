#include "global.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

extern "C" {
#include "usbdrv.h"
}
#include "i2c.h"
#include "lcd.h"
#include "analog.h"
#include "buttons.h"
#include "menu.h"
#include "delay.h"
#include "killswitch.h"
#include "usb_request.h"

//
static const bool WATCHDOG_ENABLE = false;

int main(void) {
	if (WATCHDOG_ENABLE) {
		wdt_enable(WDTO_1S);
	}

	killswitch::init();
	usbInit();
	usbDeviceDisconnect();

	i2c::init();
	lcd::init();
	buttons::init();
	menu::init();

	_delay_ms(250);

	usbDeviceConnect();
	sei();
	analog::init();

	for (;;) {
		if (WATCHDOG_ENABLE) {
			wdt_reset();
		}

		usbPoll();

		usb::executeCommandsFromUSB();

		//menu::poll();
		//delay::waitMs(150);
	}
}
