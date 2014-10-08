/*
 * Configuration.hpp
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
#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#include <string>
#include <stdexcept>
#include <map>

#include <boost/noncopyable.hpp>

namespace log4cpp {

class Properties: public std::map<std::string, std::string> {

public:

	Properties();

	virtual ~Properties();

	virtual void load(std::istream& in);

	virtual void save(std::ostream& out);

	virtual int getInt(const std::string& property, int defaultValue);

	virtual bool getBool(const std::string& property, bool defaultValue);

	virtual std::string getString(const std::string& property,

	const char* defaultValue);

protected:

	virtual void _substituteVariables(std::string& value);

};

}

namespace config {

class ConfigException: public std::runtime_error {
public:
	ConfigException(const std::string& message)
			: std::runtime_error(message) {
	}
};

class Configuration: boost::noncopyable {
public:
	static Configuration& create();
	static Configuration& create(const std::string fileName);

	static Configuration& get();

	void checkKey(const std::string& property);

	log4cpp::Properties prop;

private:
	Configuration();
	Configuration(const std::string fileName);

	static Configuration* instance;
};

int getInt(const std::string& property);

bool getBool(const std::string& property);

std::string getString(const std::string& property);

} /* namespace config */

#endif /* CONFIGURATION_HPP_ */
