#pragma once

#include "Configuration.hpp"
#include "IRequestProcessor.hpp"
#include "WifiInfo.hpp"

#include <minijson_writer.hpp>

namespace os {
    class OSInformationProcessor : public processing::IRequestProcessor, public wallaroo::Part {
    public:
        OSInformationProcessor();

        ~OSInformationProcessor();

        virtual void process(processing::Request &req, minijson::object_writer &response);

    private:
        log4cpp::Category &logger;

        wallaroo::Collaborator<common::config::Configuration> config;
        wallaroo::Collaborator<IWifiInfo> wifiInfo;
    };
}
