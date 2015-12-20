#pragma once

#include "Configuration.hpp"
#include "IoServiceProvider.hpp"
#include "GripperImageSource.hpp"
#include "JpegEncoder.hpp"

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/device.h>

#include <stdexcept>
#include <memory>

namespace camera {

    class NetworkException : public std::runtime_error {
    public:
        NetworkException(const std::string &message)
                : std::runtime_error(message) {
        }
    };

    class INetworkServer : boost::noncopyable {
    public:
        virtual ~INetworkServer() = default;
    };

    class NetworkServer : public wallaroo::Device, public INetworkServer {
    public:
        NetworkServer();

        NetworkServer(int port);

        ~NetworkServer();

    private:
        log4cpp::Category &logger;

        wallaroo::Plug<common::config::Configuration> config;
        wallaroo::Plug<camera::IImageSource> imageSource;
        wallaroo::Plug<camera::IJpegEncoder> jpegEncoder;
        wallaroo::Plug<common::IoServiceProvider> ioServiceProvider;

        int port;
        std::unique_ptr<boost::asio::ip::udp::socket> udpSocket;
        boost::asio::ip::udp::endpoint endpoint;

        std::unique_ptr<char> recvBuffer;

        void Init();

        void doReceive();
    };
}
