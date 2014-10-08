#ifndef _NETWORKSERVER_HPP_
#define _NETWORKSERVER_HPP_

#include <boost/noncopyable.hpp>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>
#include <boost/asio.hpp>
#include <stdexcept>
#include <memory>

#include "CameraImageCombiner.hpp"

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

		wallaroo::Plug<camera::IImageCombiner> imageSource;

		boost::asio::io_service ioService;
		boost::asio::ip::udp::socket udpSocket;
		boost::asio::ip::udp::endpoint endpoint;

		std::unique_ptr<char> recvBuffer;

		void doReceive();

		void initializeServer(int server);
	};
}

#endif