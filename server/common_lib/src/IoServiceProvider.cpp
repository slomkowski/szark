#include "IoServiceProvider.hpp"

#include <iostream>

namespace common {
    WALLAROO_REGISTER(IoServiceProvider)

    IoServiceProvider::IoServiceProvider()
            : signals(ioContext, SIGINT, SIGTERM) {
        signals.async_wait(std::bind(&IoServiceProvider::signalHandler, this));
    }

    boost::asio::io_context &IoServiceProvider::getIoContext() {
        return ioContext;
    }

    void IoServiceProvider::run() {
        ioContext.run();
    }

    void IoServiceProvider::signalHandler() {
        std::cerr << "Ctrl+C pressed, stopping.\n";
        ioContext.stop();
    }
}
