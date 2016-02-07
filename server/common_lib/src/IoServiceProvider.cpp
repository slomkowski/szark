#include "IoServiceProvider.hpp"

#include <iostream>

namespace common {
    WALLAROO_REGISTER(IoServiceProvider)

    IoServiceProvider::IoServiceProvider()
            : signals(ioService, SIGINT, SIGTERM) {
        signals.async_wait(std::bind(&IoServiceProvider::signalHandler, this));
    }

    boost::asio::io_service &common::IoServiceProvider::getIoService() {
        return ioService;
    }

    void IoServiceProvider::run() {
        ioService.run();
    }

    void IoServiceProvider::signalHandler() {
        std::cerr << "Ctrl+C pressed, stopping.\n";
        ioService.stop();
    }
}
