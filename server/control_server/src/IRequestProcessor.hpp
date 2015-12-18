#pragma once

#include <json/value.h>
#include <boost/asio.hpp>

namespace processing {

    class IRequestProcessor {
    public:
        /*
         * Takes the JSON document tree and parses the request. Returns the response.
         * The function is blocking till the values are gathered.
         */
        virtual void process(Json::Value &request, boost::asio::ip::address address, Json::Value &response) = 0;

        virtual ~IRequestProcessor() = default;
    };

} /* namespace processing */
