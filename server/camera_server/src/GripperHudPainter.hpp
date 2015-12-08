#pragma once

#include "Configuration.hpp"
#include "Painter.hpp"

#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

namespace camera {
    class GripperHudPainter : public IPainter, public wallaroo::Device {
    public:
        GripperHudPainter();

        virtual ~GripperHudPainter();

        virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg);

    private:
        log4cpp::Category &logger;
        wallaroo::Plug<common::config::Configuration> config;
    };
}
