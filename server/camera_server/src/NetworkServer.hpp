#pragma once

#include "Configuration.hpp"
#include "IoServiceProvider.hpp"
#include "GripperImageSource.hpp"
#include "JpegEncoder.hpp"

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/part.h>

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

    constexpr unsigned int UDP_MAX_PAYLOAD_SIZE = 65506;

    constexpr unsigned int SEND_BUFFER_SIZE = 0x20000;

    class NetworkServer : public wallaroo::Part, public INetworkServer {
    public:
        NetworkServer();

        NetworkServer(int port);

        ~NetworkServer();

    private:
        log4cpp::Category &logger;

        wallaroo::Collaborator<common::config::Configuration> config;
        wallaroo::Collaborator<camera::IImageSource> imageSource;
        wallaroo::Collaborator<camera::IJpegEncoder> jpegEncoder;
        wallaroo::Collaborator<common::IoServiceProvider> ioServiceProvider;

        int port;
        std::unique_ptr<boost::asio::ip::udp::socket> udpSocket;
        boost::asio::ip::udp::endpoint endpoint;

        std::unique_ptr<char> recvBuffer;

        unsigned char *sendBuffer = nullptr;
        unsigned char *sendImgBuffer = nullptr;

        void Init();

        void doReceive();
    };
}
