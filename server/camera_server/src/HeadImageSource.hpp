#pragma once

#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"
#include <Configuration.hpp>

#include <opencv2/opencv.hpp>
#include <wallaroo/part.h>
#include <log4cpp/Category.hh>

namespace camera {

    class HeadImageSource : public IImageSource, public wallaroo::Part {
    public:
        HeadImageSource();

        virtual ~HeadImageSource();

        virtual cv::Mat getImage(bool drawHud);

    private:
        log4cpp::Category &logger;

        wallaroo::Collaborator<common::config::Configuration> config;
        wallaroo::Collaborator<IImageGrabber> cameraGrabber;

        wallaroo::Collaborator<IPainter> hudPainter;
    };
}
