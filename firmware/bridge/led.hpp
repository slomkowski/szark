/*
 * led.hpp
 *
 *  Project: bridge
 *  Created on: 18 mar 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */
#ifndef LED_HPP_
#define LED_HPP_

namespace led {
	void init();

	enum Diode {
		GREEN, YELLOW
	};

	/**
	 * Sets the state of the diode.
	 * @param diode
	 * @param enabled - true for LED lighted up.
	 */
	void setState(Diode diode, bool enabled);

	/**
	 * Set state for all diodes.
	 * @param enabled
	 */
	void setState(bool enabled);

	/**
	 * Toggle the state of the diode.
	 * @param diode
	 */
	void toggleState(Diode diode);
}

#endif /* LED_HPP_ */
