/*
 * NetServer.hpp
 *
 *  Project: server
 *  Created on: 5 maj 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */
#ifndef NETSERVER_HPP_
#define NETSERVER_HPP_

#include <string>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>
#include <boost/asio.hpp>

#include "RequestQueuer.hpp"

namespace processing {

class NetException: public std::runtime_error {
public:
	NetException(const std::string& message)
			: std::runtime_error(message) {
	}
};

class INetServer {
public:
	virtual void sendResponse(long id, std::string response, bool transmit) = 0;

	virtual void run() = 0;

	virtual ~INetServer() = default;
};

class NetServer: public wallaroo::Device, public INetServer {
public:
	NetServer();

	NetServer(unsigned int port);

	virtual void run();

	virtual void sendResponse(long id, std::string response, bool transmit);

	virtual ~NetServer();

private:
	log4cpp::Category& logger;

	wallaroo::Plug<IRequestQueuer> reqQueuer;

	boost::asio::io_service ioService;
	boost::asio::ip::udp::socket udpSocket;
	boost::asio::ip::udp::endpoint recvSenderEndpoint;

	std::unordered_map<long, boost::asio::ip::udp::endpoint> sendersMap;

	short udpPort;

	std::unique_ptr<char> buff;

	void doReceive();
};

} /* namespace processing */

#endif /* NETSERVER_HPP_ */
