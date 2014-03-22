/*
 * usb_support.cpp
 *
 *  Project: bridge
 *  Created on: 21 mar 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include <LUFA/Drivers/USB/USB.h>

// LUFA defines it
#undef putc

#include "led.hpp"
#include "usb_support.hpp"
#include "usb_descriptors.h"
#include "usb_processcommands.hpp"

static_assert(usb::BUFFER_SIZE == 2 * VENDOR_IO_EPSIZE, "USB request buffer size must match 2 * endpoint size.");

void usb::init() {
	led::setState(led::YELLOW, false);

	USB_Init();
}

static uint8_t buffer[2 * VENDOR_IO_EPSIZE];

void usb::poll() {
	USB_USBTask();

	Endpoint_SelectEndpoint(VENDOR_OUT_EPADDR);
	if (Endpoint_IsOUTReceived()) {
		led::setState(led::YELLOW, false);

		Endpoint_Read_Stream_LE(buffer, VENDOR_IO_EPSIZE, NULL);
		Endpoint_Read_Stream_LE(buffer + VENDOR_IO_EPSIZE, VENDOR_IO_EPSIZE, NULL);

		Endpoint_ClearOUT();

		inBuff.init();
		usb::inBuff.push(buffer, BUFFER_SIZE);

		usb::executeCommandsFromUSB();

		Endpoint_SelectEndpoint(VENDOR_IN_EPADDR);
		Endpoint_Write_Stream_LE(usb::outBuff.data, VENDOR_IO_EPSIZE, NULL);
		//Endpoint_Write_Stream_LE(usb::outBuff.data + VENDOR_IO_EPSIZE, VENDOR_IO_EPSIZE, NULL);
		Endpoint_ClearIN();

		led::setState(led::YELLOW, true);
	}
}

void EVENT_USB_Device_Connect() {
	led::setState(led::YELLOW, true);
}

void EVENT_USB_Device_Disconnect() {
	led::setState(led::YELLOW, false);
}

void EVENT_USB_Device_ConfigurationChanged() {
	bool ConfigSuccess = true;

	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_IN_EPADDR, EP_TYPE_BULK, VENDOR_IO_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_OUT_EPADDR, EP_TYPE_BULK, VENDOR_IO_EPSIZE, 1);

	led::setState(led::GREEN, ConfigSuccess);
}

void EVENT_USB_Device_ControlRequest() {

}
