#include "global.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

extern "C" {
#include "usbdrv.h"
}
#include "i2c.h"
#include "lcd.h"
#include "motor_driver.h"
#include "analog.h"
#include "buttons.h"
#include "menu.h"

//
static const bool WATCHDOG_ENABLE = false;

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *) data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR) {

	} else {
		/* calss requests USBRQ_HID_GET_REPORT and USBRQ_HID_SET_REPORT are
		 * not implemented since we never call them. The operating system
		 * won't call them either because our descriptor defines no meaning.
		 */
	}
	return 0; /* default for not implemented requests: return no data back to host */
}

int main(void) {
	if (WATCHDOG_ENABLE) {
		wdt_enable(WDTO_1S);
	}

	usbInit();
	usbDeviceDisconnect();

	i2c::init();
	lcd::init();
	analog::init();
	buttons::init();
	menu::init();

	for (uint8_t i = 0; i < 0xff; i++) {
		if (WATCHDOG_ENABLE) {
			wdt_reset();
		}
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();

	for (;;) {
		if (WATCHDOG_ENABLE) {
			wdt_reset();
		}

		usbPoll();

		menu::poll();
		_delay_ms(100);
	}
}
