#pragma once

#include "usb-commands.hpp"

extern "C" {
#include <libusb.h>
}

#include <boost/noncopyable.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>

#include <stdexcept>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

namespace bridge {
/**
* This exception is thrown if an USB communication error occurs. You can view the error description
* by calling exception.what()
*/
    class CommException : public std::runtime_error {
    public:
        CommException(const std::string &message)
                : std::runtime_error(message) {
        }
    };

    class ICommunicator {
    public:
        virtual ~ICommunicator() = default;

        virtual void sendData(std::vector<uint8_t> &data) = 0;

        virtual std::vector<uint8_t> receiveData() = 0;
    };

    class USBCommunicator : public ICommunicator, public wallaroo::Device {
    public:
        USBCommunicator();

        virtual ~USBCommunicator();

        void sendData(std::vector<uint8_t> &data);

        std::vector<uint8_t> receiveData();

    private:
        log4cpp::Category &logger;
        libusb_device_handle *devHandle;
    };

} /* namespace USB */
