/*
 * NetServer.cpp
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
#include <boost/format.hpp>

#include "NetServer.hpp"
#include "Configuration.hpp"

namespace processing {

const int MAX_PACKET_SIZE = 1200;

using namespace std;
using namespace boost;

WALLAROO_REGISTER(NetServer);

using namespace std;
using boost::asio::ip::udp;

NetServer::NetServer()
		:
				logger(log4cpp::Category::getInstance("NetServer")),
				reqQueuer("requestQueuer", RegistrationToken()),
				ioService(),
				receiverSocket(ioService),
				senderSocket(ioService) {

	buff.reset(new char[MAX_PACKET_SIZE]);

	udpPort = config::getInt("szark.server.NetServer.port");

	logger.notice((format("Opening sockets with port %u.") % udpPort).str());

	receiverSocket.open(udp::v4());
	senderSocket.open(udp::v4());

	system::error_code err;
	receiverSocket.bind(udp::endpoint(udp::v4(), udpPort), err);
	if (err) {
		throw NetException("error at binding socket: " + err.message());
	}

	doReceive();

	logger.notice("Instance created.");
}

void NetServer::doReceive()
{
	receiverSocket.async_receive_from(asio::buffer(buff.get(), MAX_PACKET_SIZE), recvSenderEndpoint,
			[&](system::error_code ec, size_t bytes_recvd) {
				if (!ec && bytes_recvd > 0) {
					logger.debug((format("Received %u bytes.") % bytes_recvd).str());

					auto msg = string(buff.get(), 0, bytes_recvd);
					long id = reqQueuer->addRequest(msg);
					if(id != INVALID_MESSAGE) {
						logger.debug((format("Adding key %l to senders map.") % id).str());
						sendersMap[id] = recvSenderEndpoint;
					}
				} else {
					logger.error((format("Error when receiving packet (%u bytes): %s.") % bytes_recvd % ec.message() ).str());
				}
				doReceive();
			});
}

void NetServer::sendResponse(long id, std::string response, bool transmit) {
	if (sendersMap.find(id) == sendersMap.end()) {
		throw NetException((format("senders map doesn't contain key: %l.") % id).str());
	}
	// 	TODO to zrobić thread-safe
	auto senderEndpoint = sendersMap[id];

	logger.debug((format("Removing key %l from senders map.") % id).str());
	sendersMap.erase(id);

	if (transmit == true) {
		logger.info((format("Sending response (length %d) to %s.") %
				response.length() % senderEndpoint.address().to_string()).str());

		unsigned int bytes_sent = senderSocket.send_to(asio::buffer(response), senderEndpoint);

		if (bytes_sent != response.length()) {
			throw NetException(
					(format("wrong amount of data sent: %u instead of %u") % bytes_sent % response.length()).str());
		}
	}
}

void NetServer::run() {
	logger.notice((format("Starting UDP listener on port %u.") % udpPort).str());

	system::error_code err;
	ioService.run();
}

NetServer::~NetServer() {
	logger.notice("Instance destroyed.");
}

} /* namespace processing */
