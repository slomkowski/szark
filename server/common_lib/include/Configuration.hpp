#pragma once

#include <wallaroo/registered.h>

#include <boost/noncopyable.hpp>

#include <string>
#include <stdexcept>

namespace common {
    namespace config {

        class ConfigException : public std::runtime_error {
        public:
            ConfigException(const std::string &message)
                    : std::runtime_error(message) {
            }
        };

        struct ConfigurationImpl;

        class Configuration : boost::noncopyable, public wallaroo::Part {
        public:
            Configuration();

            Configuration(const std::string fileName);

            Configuration(const std::vector<std::string> fileNames);

            ~Configuration();

            void putInt(const std::string &property, const int val);

            void putBool(const std::string &property, const bool val);

            void putString(const std::string &property, const std::string val);

            int getInt(const std::string &property);

            bool getBool(const std::string &property);

            std::string getString(const std::string &property);

        private:
            ConfigurationImpl *impl;
        };
    }
}
