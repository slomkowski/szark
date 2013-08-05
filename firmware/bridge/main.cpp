#include "global.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

extern "C" {
#include "usbdrv.h"
}

PROGMEM const uint8_t usbHidReportDescriptor[22] = { /* USB report descriptor */
0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
	0x09, 0x01,                    // USAGE (Vendor Usage 1)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x00,                    //   USAGE (Undefined)
	0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
	0xc0                           // END_COLLECTION
	};
/* The descriptor above is a dummy only, it silences the drivers. The report
 * it describes consists of one byte of undefined data.
 * We don't transfer our data through HID reports, we use custom requests
 * instead.
 */

/* ------------------------------------------------------------------------- */

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
	uchar i;

	wdt_enable(WDTO_1S);

	usbInit();
	usbDeviceDisconnect();
	i = 0;
	while (--i) {
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();
	for (;;) {
		wdt_reset();
		usbPoll();
	}
}

/* ------------------------------------------------------------------------- */
