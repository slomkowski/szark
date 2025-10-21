#pragma once

#include <wallaroo/registered.h>

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

namespace common {
    class IoServiceProvider : boost::noncopyable, public wallaroo::Part {
    public:
        IoServiceProvider();

        boost::asio::io_context &getIoContext();

        void run();

    private:
        boost::asio::io_context ioContext;
        boost::asio::signal_set signals;

        void signalHandler();
    };
}
