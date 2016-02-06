#pragma once

#include <wallaroo/registered.h>

#include <boost/asio.hpp>

namespace common {
    class IoServiceProvider : boost::noncopyable, public wallaroo::Part {
    public:
        IoServiceProvider();

        boost::asio::io_service &getIoService();

        void run();

    private:
        boost::asio::io_service ioService;
        boost::asio::signal_set signals;

        void signalHandler();
    };
}
