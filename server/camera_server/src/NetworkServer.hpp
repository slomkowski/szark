#ifndef _NETWORKSERVER_HPP_
#define _NETWORKSERVER_HPP_

#include <stdexcept>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/device.h>

#include "GripperImageSource.hpp"

namespace camera {

	class NetworkException : public std::runtime_error {
	public:
		NetworkException(const std::string &message)
				: std::runtime_error(message) {
		}
	};

	class INetworkServer : boost::noncopyable {
	public:
		virtual void run() = 0;

		virtual ~INetworkServer() = default;
	};

	class NetworkServer : public wallaroo::Device, public INetworkServer {
	public:
		NetworkServer();

		NetworkServer(int port);

		~NetworkServer();

		virtual void run();

	private:
		log4cpp::Category &logger;

		wallaroo::Plug<camera::IImageSource> imageSource;

		boost::asio::io_service ioService;
		boost::asio::ip::udp::socket udpSocket;
		boost::asio::ip::udp::endpoint endpoint;

		std::unique_ptr<char> recvBuffer;

		void doReceive();

		void initializeServer(int server);
	};
}

#endif