#pragma once

#include "Configuration.hpp"
#include "Painter.hpp"

#include <wallaroo/part.h>
#include <log4cpp/Category.hh>

namespace camera {
    class GripperHudPainter : public IPainter, public wallaroo::Part {
    public:
        GripperHudPainter();

        virtual ~GripperHudPainter();

        virtual cv::Mat drawContent(cv::Mat rawImage, boost::any additionalArg);

    private:
        log4cpp::Category &logger;
        wallaroo::Collaborator<common::config::Configuration> config;
    };
}
