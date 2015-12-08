#pragma once

#include <opencv2/opencv.hpp>
#include <wallaroo/device.h>
#include <log4cpp/Category.hh>
#include <Configuration.hpp>

#include "ImageSource.hpp"
#include "Painter.hpp"
#include "CameraImageGrabber.hpp"

namespace camera {

    class HeadImageSource : public IImageSource, public wallaroo::Device {
    public:
        HeadImageSource();

        virtual ~HeadImageSource();

        virtual cv::Mat getImage(bool drawHud);

        virtual void getEncodedImage(bool drawHud, EncodedImageProcessor processor);

    private:
        log4cpp::Category &logger;

        bool leftCameraIsFaster;

        std::vector<int> jpegEncoderParameters;

        wallaroo::Plug<common::config::Configuration> config;
        wallaroo::Plug<IImageGrabber> cameraGrabber;

        wallaroo::Plug<IPainter> hudPainter;
    };
}
