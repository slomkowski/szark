#include "global.h"
#include "usbdrv.h"
#include "requests.h"

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *) data;

	static uint16_t result;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_VENDOR) {
		return 0;
	}

	usbMsgPtr = (unsigned short) &result;


	return 0; /* default for not implemented requests: return no data back to host */
}
