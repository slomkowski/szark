#pragma once

#include "Interface.hpp"

#include <wallaroo/registered.h>

#include <stdexcept>

namespace common {
    namespace bridge {

        class InterfaceProviderException : public std::runtime_error {
        public:
            InterfaceProviderException(const std::string &message)
                    : std::runtime_error(message) {
            }
        };

        struct _InterfaceProviderImpl;

        class InterfaceProvider : boost::noncopyable, public wallaroo::Part {
        public:
            InterfaceProvider(const bool &isMaster);

            virtual void Init() override;

            virtual ~InterfaceProvider();

            virtual bool isValid();

            virtual Interface *getInterface();

        private:
            _InterfaceProviderImpl *impl;
        };
    }
}
