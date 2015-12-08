#pragma once

#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"

#include <Configuration.hpp>

#include <opencv2/opencv.hpp>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>

namespace camera {

    class GripperImageSource : public IImageSource, public wallaroo::Device {
    public:
        GripperImageSource();

        virtual ~GripperImageSource();

        virtual cv::Mat getImage(bool drawHud);

    private:
        log4cpp::Category &logger;

        bool leftCameraIsFaster;

        wallaroo::Plug<common::config::Configuration> config;
        wallaroo::Plug<IImageGrabber> leftCameraGrabber;
        wallaroo::Plug<IImageGrabber> rightCameraGrabber;

        wallaroo::Plug<IPainter> hudPainter;
    };
}
