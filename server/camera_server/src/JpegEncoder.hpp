#pragma once

#include "utils.hpp"

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <opencv2/opencv.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/device.h>

#include <turbojpeg.h>

#include <vector>
#include <cstring>

namespace camera {

    class IJpegEncoder : boost::noncopyable {
    public:
        virtual ~IJpegEncoder() = default;

        virtual unsigned int encodeImage(cv::Mat inputImage,
                                         unsigned char *outputBuffer,
                                         unsigned int maxOutputLength) = 0;
    };

    class OpenCvJpegEncoder : public IJpegEncoder, public wallaroo::Device {
    public:

        unsigned int encodeImage(cv::Mat inputImage,
                                 unsigned char *outputBuffer,
                                 unsigned int maxOutputLength);

    private:
        log4cpp::Category &logger = log4cpp::Category::getInstance("OpenCvJpegEncoder");
    };

    class TurboJpegEncoder : public IJpegEncoder, public wallaroo::Device {
    public:
        TurboJpegEncoder();

        ~TurboJpegEncoder();

        unsigned int encodeImage(cv::Mat inputImage,
                                 unsigned char *outputBuffer,
                                 unsigned int maxOutputLength);

    private:
        log4cpp::Category &logger = log4cpp::Category::getInstance("TurboJpegEncoder");

        tjhandle _jpegCompressor;
    };
}