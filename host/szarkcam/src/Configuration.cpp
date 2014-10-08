/*
 * Configuration.cpp
 *
 *  Project: server
 *  Created on: 24 kwi 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#include <fstream>

#include "Configuration.hpp"

using namespace config;

Configuration* config::Configuration::instance = nullptr;

Configuration& config::Configuration::create() {
	if (instance != nullptr) {
		throw ConfigException("Configuration instance already exists");
	}

	instance = new Configuration();
	return *instance;
}

Configuration& config::Configuration::create(const std::string fileName) {
	if (instance != nullptr) {
		throw ConfigException("Configuration instance already exists");
	}

	instance = new Configuration(fileName);
	return *instance;
}

Configuration& config::Configuration::get() {
	return *instance;
}

config::Configuration::Configuration() {
	std::ifstream conf;
	//TODO wyjątki itd.
	conf.open("logger.properties");
	prop.load(conf);
	conf.close();
}

config::Configuration::Configuration(const std::string fileName) {
//TODO konstruktor z nazwą pliku
}

int config::getInt(const std::string& property) {
	auto& c = Configuration::get();
	c.checkKey(property);
	return c.prop.getInt(property, 0);
}

bool config::getBool(const std::string& property) {
	auto& c = Configuration::get();
	c.checkKey(property);
	return c.prop.getBool(property, false);
}

std::string config::getString(const std::string& property) {
	auto& c = Configuration::get();
	c.checkKey(property);
	return c.prop.getString(property, "");
}

void config::Configuration::checkKey(const std::string& property) {
	if (prop.find(property) == prop.end()) {
		throw ConfigException(std::string("key \'") + property + "\' not found");
	}
}
