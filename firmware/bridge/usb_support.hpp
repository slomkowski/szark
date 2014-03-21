/*
 * usb_support.hpp
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
#ifndef USB_SUPPORT_HPP_
#define USB_SUPPORT_HPP_

namespace usb {
	void init();

	void poll();
}

#endif /* USB_SUPPORT_HPP_ */
