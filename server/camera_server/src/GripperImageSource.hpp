#pragma once

#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"

#include <Configuration.hpp>

#include <opencv2/opencv.hpp>
#include <wallaroo/part.h>
#include <log4cpp/Category.hh>

namespace camera {

    class GripperImageSource : public IImageSource, public wallaroo::Part {
    public:
        GripperImageSource();

        virtual ~GripperImageSource();

        virtual cv::Mat getImage(bool drawHud);

    private:
        log4cpp::Category &logger;

        bool leftCameraIsFaster;

        wallaroo::Collaborator<common::config::Configuration> config;
        wallaroo::Collaborator<IImageGrabber> leftCameraGrabber;
        wallaroo::Collaborator<IImageGrabber> rightCameraGrabber;

        wallaroo::Collaborator<IPainter> hudPainter;
    };
}
