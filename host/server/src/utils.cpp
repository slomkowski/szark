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
#include <sstream>

std::chrono::microseconds utils::measureTime(std::function<void()> func) {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	using std::chrono::high_resolution_clock;

	auto timerBegin = high_resolution_clock::now();

	func();

	auto timerEnd = high_resolution_clock::now();

	return duration_cast<microseconds>(timerEnd - timerBegin);
}

std::string utils::getTimestamp() {

	std::stringstream now;

	auto tp = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	size_t modulo = ms.count() % 1000;

	time_t seconds = std::chrono::duration_cast<std::chrono::seconds>(ms).count();

	char buffer[25]; // holds "2013-12-01 21:31:42"

	// note: localtime() is not threadsafe, lock with a mutex if necessary
	if (strftime(buffer, 25, "%H:%M:%S.", localtime(&seconds))) {
		now << buffer;
	}

	// ms
	now.fill('0');
	now.width(3);
	now << modulo;

	return now.str();
}
