#pragma once

#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"
#include <Configuration.hpp>

#include <opencv2/opencv.hpp>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

namespace camera {

    class HeadImageSource : public IImageSource, public wallaroo::Device {
    public:
        HeadImageSource();

        virtual ~HeadImageSource();

        virtual cv::Mat getImage(bool drawHud);

    private:
        log4cpp::Category &logger;

        wallaroo::Plug<common::config::Configuration> config;
        wallaroo::Plug<IImageGrabber> cameraGrabber;

        wallaroo::Plug<IPainter> hudPainter;
    };
}
