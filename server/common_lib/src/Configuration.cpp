#include <fstream>

#include "wallaroo/dynamic_lib.h"

#include "Configuration.hpp"

using namespace common::config;

WALLAROO_REGISTER(Configuration, std::string);

common::config::Configuration::Configuration(const std::string fileName) {
	std::ifstream conf;
	//TODO wyjątki itd.
	conf.open(fileName);
	prop.load(conf);
	conf.close();
}

int common::config::Configuration::getInt(const std::string &property) {
	checkKey(property);
	return prop.getInt(property, 0);
}

bool common::config::Configuration::getBool(const std::string &property) {
	checkKey(property);
	return prop.getBool(property, false);
}

std::string common::config::Configuration::getString(const std::string &property) {
	checkKey(property);
	return prop.getString(property, "");
}

void common::config::Configuration::checkKey(const std::string &property) {
	if (prop.find(property) == prop.end()) {
		throw ConfigException(std::string("key \'") + property + "\' not found");
	}
}
