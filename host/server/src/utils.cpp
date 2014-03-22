/*
 * utils.cpp
 *
 *  Project: server
 *  Created on: 22 mar 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */
#include "utils.hpp"

#include <chrono>

std::chrono::microseconds utils::measureTime(std::function<void()> func) {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	using std::chrono::high_resolution_clock;

	auto timerBegin = high_resolution_clock::now();

	func();

	auto timerEnd = high_resolution_clock::now();

	return duration_cast<microseconds>(timerEnd - timerBegin);
}
