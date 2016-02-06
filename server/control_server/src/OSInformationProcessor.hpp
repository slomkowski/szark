#pragma once

#include "Configuration.hpp"
#include "IRequestProcessor.hpp"
#include "WifiInfo.hpp"

#include <minijson_writer.hpp>

namespace os {
    class OSInformationProcessor : public processing::IRequestProcessor, public wallaroo::Device {
    public:
        OSInformationProcessor();

        ~OSInformationProcessor();

        virtual void process(Json::Value &request, boost::asio::ip::address address,
                             minijson::object_writer &response) override;

    private:
        log4cpp::Category &logger;

        wallaroo::Plug<common::config::Configuration> config;
        wallaroo::Plug<IWifiInfo> wifiInfo;
    };
}
