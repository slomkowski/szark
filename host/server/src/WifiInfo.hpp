/*
 * WifiInfo.hpp
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
#ifndef WIFIINFO_HPP_
#define WIFIINFO_HPP_

#include <string>
#include <stdexcept>

#include <boost/noncopyable.hpp>
#include <log4cpp/Category.hh>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/wireless.h>

namespace net {

class WifiException: public std::runtime_error {
public:
	WifiException(const std::string& message)
			: std::runtime_error(message) {
	}
};

class WifiInfo: boost::noncopyable {
public:
	/*
	 * The constructor checks if the device is valid.
	 */
	WifiInfo(std::string iwName);

	/*
	 * Default constructor reads the device name from config space.
	 */
	WifiInfo();

	~WifiInfo();

	/*
	 * Returns signal level in dBm.
	 */
	double getSignalLevel();

	/*
	 * Returns the bitrate of the link if MBits/s.
	 */
	double getBitrate();

	/*
	 * Returns wireless interface name.
	 */
	std::string getInterfaceName() {
		return iwName;
	}

private:
	log4cpp::Category& logger;
	std::string iwName;

	int sockfd;
	bool enabled = true;
	iw_statistics stats;
	iwreq req;

	void prepareStructs();
	void init();
};

} /* namespace net */

#endif /* WIFIINFO_HPP_ */
