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
#include <functional>
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
				udpSocket(ioService) {

	buff.reset(new char[MAX_PACKET_SIZE]);

	udpPort = config::getInt("szark.server.NetServer.port");

	logger.notice((format("Opening listener socket with port %u.") % udpPort).str());

	system::error_code err;
	udpSocket.open(udp::v4());
	udpSocket.bind(udp::endpoint(udp::v4(), udpPort), err);
	if (err) {
		throw NetException("error at binding socket: " + err.message());
	}

	doReceive();

	logger.notice("Instance created.");
}

void NetServer::doReceive()
{
	udpSocket.async_receive_from(asio::buffer(buff.get(), MAX_PACKET_SIZE), recvSenderEndpoint,
			[this](system::error_code ec, size_t bytes_recvd) {
				if (!ec && bytes_recvd > 0) {
					logger.info((format("Received %d bytes from %s.") % bytes_recvd % recvSenderEndpoint.address().to_string()).str());

					if(recvSenderEndpoint.protocol() != udp::v4()) {
						logger.error("Received address is not an IPv4 address: " + recvSenderEndpoint.address().to_string());
					} else {
						auto msg = string(buff.get(), 0, bytes_recvd);
						logger.debug(string("Received data: ") + msg);

						long id = reqQueuer->addRequest(msg);

						if(id != INVALID_MESSAGE) {
							logger.debug((format("Adding key %ld to senders map.") % id).str());

							sendersMap[id] = recvSenderEndpoint;

							logger.debug((format("Senders map contains now %d keys.") % sendersMap.size()).str());
						}

						using namespace std::placeholders;
						reqQueuer->setResponseSender(bind(&NetServer::sendResponse, this, _1, _2, _3));
					}
				} else {
					logger.error((format("Error when receiving packet (%u bytes): %s.") % bytes_recvd % ec.message() ).str());
				}
				doReceive();
			});
}

void NetServer::sendResponse(long id, std::string response, bool transmit) {
	const auto senderEndpoint = sendersMap.find(id);

	if (senderEndpoint == sendersMap.end()) {
		throw NetException((format("senders map doesn't contain key: %ld.") % id).str());
	}

	if (transmit == true) {
		udp::endpoint endpoint = senderEndpoint->second;

		unsigned int responseLength = response.length();

		logger.info((format("Sending response (length %d) to %s.") %
				responseLength % endpoint.address().to_string()).str());

		logger.debug(string("Sending data: ") + response);

		udpSocket.async_send_to(asio::buffer(response), endpoint,
				[responseLength](system::error_code ec, size_t bytes_sent)
				{
					if(ec) {
						throw NetException("error when sending response: " + ec.message());
					}
					if (bytes_sent != responseLength) {
						throw NetException(
								(format("wrong amount of data sent: %u instead of %u") % bytes_sent % responseLength).str());
					}
				});
	}

	ioService.post([id,this]() {
		logger.debug((format("Removing key %ld from senders map.") % id).str());
		sendersMap.erase(id);
		logger.debug((format("Senders map contains now %d keys.") % sendersMap.size()).str());
	});
}

void NetServer::run() {
	logger.notice(
			(format("Starting UDP listener on port %u.") % udpPort).str());

	system::error_code err;
	ioService.run();
}

NetServer::~NetServer() {
	logger.notice("Instance destroyed.");
}

} /* namespace processing */
