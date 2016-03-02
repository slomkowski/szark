#pragma once

#include <minijson_writer.hpp>

#include <boost/asio.hpp>

#include <memory>

namespace processing {

    struct Request {
        long internalId = -1;
        long serial = -1;
        bool skipResponse = false;
        boost::asio::ip::address ipAddress;
        std::string reqJson;
        std::string sendTimestamp;
        std::string receiveTimestamp;
    };

    class IRequestProcessor {
    public:
        /*
         * Takes the JSON document tree and parses the request. Returns the response.
         * The function is blocking till the values are gathered.
         */
        virtual void process(Request &req, minijson::object_writer &response) = 0;

        virtual ~IRequestProcessor() = default;
    };

}
