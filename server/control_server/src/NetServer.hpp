#pragma once

#include "RequestQueuer.hpp"
#include "Configuration.hpp"

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>
#include <boost/asio.hpp>

#include <string>
#include <memory>
#include <unordered_map>
#include <stdexcept>

namespace processing {

    class NetException : public std::runtime_error {
    public:
        NetException(const std::string &message)
                : std::runtime_error(message) {
        }
    };

    class INetServer {
    public:
        virtual void sendResponse(long id, std::string response, bool transmit) = 0;

        virtual void run() = 0;

        virtual ~INetServer() = default;
    };

    class NetServer : public wallaroo::Device, public INetServer {
    public:
        NetServer();

        NetServer(unsigned int port);

        virtual void run();

        virtual void sendResponse(long id, std::string response, bool transmit);

        virtual ~NetServer();

    private:
        log4cpp::Category &logger;

        wallaroo::Plug<common::config::Configuration> config;
        wallaroo::Plug<IRequestQueuer> reqQueuer;

        boost::asio::io_service ioService;
        boost::asio::ip::udp::socket udpSocket;
        boost::asio::ip::udp::endpoint recvSenderEndpoint;

        std::unordered_map<long, boost::asio::ip::udp::endpoint> sendersMap;

        short udpPort;

        std::unique_ptr<char> buff;

        void Init();

        void doReceive();

        void removeFromRequestMap(long id);
    };

} /* namespace processing */
