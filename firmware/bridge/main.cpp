#include "global.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

extern "C" {
#include "usbdrv.h"
}
#include "i2c.h"
#include "lcd.h"
#include "motor_driver.h"
#include "arm_driver.h"
#include "analog.h"
#include "buttons.h"
#include "menu.h"
#include "delay.h"
#include "killswitch.h"

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

	killswitch::init();
	usbInit();
	usbDeviceDisconnect();

	i2c::init();
	lcd::init();
	buttons::init();
	menu::init();

	delay::waitMs(255);

	usbDeviceConnect();
	sei();
	analog::init();

	for (;;) {
		if (WATCHDOG_ENABLE) {
			wdt_reset();
		}

		usbPoll();

		menu::poll();
		delay::waitMs(100);
	}
}
