/*
 * DataHolder.cpp
 *
 *  Project: server
 *  Created on: 13 maj 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */
#include <cstring>

#include "DataHolder.hpp"

using namespace bridge;

bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr) {
	initData(request, pr, 0);
}

bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, const uint8_t byte) {
	initData(request, pr, sizeof(char));

	data[1] = byte;
}

bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, const std::vector<uint8_t>& vec) {
	initData(request, pr, vec.size());

	unsigned int idx = 1;
	for (auto& val : vec) {
		data[idx] = val;
		idx++;
	}
}

bridge::DataHolder::DataHolder(const DataHolder &dh) {
	this->length = dh.length;
	this->priority = dh.priority;
	std::memcpy(this->data, dh.data, length + 1);
}

bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, void* data, int size) {
	initData(request, pr, size);
	std::memcpy(this->data + 1, data, length);
}

DataHolder& bridge::DataHolder::operator=(const DataHolder &dh) {
	this->length = dh.length;
	this->priority = dh.priority;
	std::memcpy(this->data, dh.data, length + 1);

	return *this;
}

bool bridge::DataHolder::equals(const DataHolder &right) {

	if (this->priority != right.priority) {
		return false;
	}

	if (this->length != right.length) {
		return false;
	}

	if (std::memcmp(this->data, right.data, this->length + 1) != 0) {
		return false;
	}

	return true;
}

void bridge::DataHolder::initData(USBCommands::Request request, const int pr, unsigned int dataSize) {
	priority = pr;
	length = dataSize + 1;
	data[0] = request;
}

