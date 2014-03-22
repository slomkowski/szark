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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "led.hpp"
#include "usb_support.hpp"
#include "usb_descriptors.h"
#include "usb_processcommands.hpp"
#include "lcd.hpp"
#include "delay.hpp"
#include "killswitch.hpp"
#include "menu.hpp"

#include "usb-settings.hpp"

static_assert(VENDOR_OUT_EPSIZE >= VENDOR_IN_EPSIZE, "Host-to-device endpoint size has to be bigger than device-to-host endpoint");
static_assert(usb::BUFFER_SIZE >= VENDOR_OUT_EPSIZE, "USB request buffer must be bigger than Host-to-device endpoint size.");

static_assert(USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE == 2 * VENDOR_OUT_EPSIZE, "OUT data packet has to contain 2 OUT endpoints");
static_assert(USB_SETTINGS_DEVICE_TO_HOST_DATAPACKET_SIZE == VENDOR_IN_EPSIZE, "IN data packet has to match IN endpoint size");

static volatile bool menuEnabled = true;

static void timerSetEnabled(bool enabled) {
	if (enabled) {
		menuEnabled = false;
		TCNT1 = 0;
		TIMSK1 |= (1 << OCIE1A);
	} else {
		TIMSK1 &= ~(1 << OCIE1A);
		menuEnabled = true;
	}
}

static void timerReset() {
	if (menuEnabled) {
		menuEnabled = false;
		lcd::clrscr();
	}
	timerSetEnabled(true);
}

ISR(TIMER1_COMPA_vect, ISR_NOBLOCK) {
	timerSetEnabled(false);
	killswitch::setActive(true);

	lcd::clrscr();
	lcd::putsp(PSTR("HOST SEVERED\nCOMMUNICATION"));
	menu::reinitMenu();
}

void EVENT_USB_Device_Connect() {
	timerSetEnabled(false);

	led::setState(led::YELLOW, false);
	led::setState(led::GREEN, false);
}

void EVENT_USB_Device_Disconnect() {
	timerSetEnabled(false);

	led::setState(led::YELLOW, false);
	led::setState(led::GREEN, false);
}

void EVENT_USB_Device_ConfigurationChanged() {
	timerSetEnabled(false);

	bool ConfigSuccess = true;

	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_IN_EPADDR, EP_TYPE_BULK, VENDOR_IN_EPSIZE, 2);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_OUT_EPADDR, EP_TYPE_BULK, VENDOR_OUT_EPSIZE, 2);

	led::setState(led::GREEN, ConfigSuccess);
}

void EVENT_USB_Device_ControlRequest() {

}

void usb::init() {
	led::setState(led::YELLOW, false);
	led::setState(led::GREEN, false);

	// set timeout timer
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS12); // CTC mode, divide clock by 1024
	OCR1A = 10000;

	USB_Init();
}

void usb::poll() {
	USB_USBTask();

	Endpoint_SelectEndpoint(VENDOR_OUT_EPADDR);
	if (Endpoint_IsOUTReceived()) {
		led::setState(led::YELLOW, true);

		timerReset();

		usb::InputBuff.length = USB_SETTINGS_HOST_TO_DEVICE_DATAPACKET_SIZE;

		Endpoint_Read_Stream_LE(usb::InputBuff.data, VENDOR_OUT_EPSIZE, NULL);
		Endpoint_Read_Stream_LE(usb::InputBuff.data + VENDOR_OUT_EPSIZE, VENDOR_OUT_EPSIZE, NULL);
		Endpoint_ClearOUT();

		usb::executeCommandsFromUSB();

		Endpoint_SelectEndpoint(VENDOR_IN_EPADDR);
		Endpoint_Write_Stream_LE(usb::OutputBuff.data, VENDOR_IN_EPSIZE, NULL);
		Endpoint_ClearIN();

		led::setState(led::YELLOW, false);
	}
}

bool usb::isMenuEnabled() {
	return menuEnabled;
}
