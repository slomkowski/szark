#pragma once

#include "utils.hpp"

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <opencv2/opencv.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/part.h>

#include <turbojpeg.h>

#include <vector>
#include <cstring>

namespace camera {

    const int DEFAULT_JPEG_QUALITY = 45;

    class IJpegEncoder : boost::noncopyable {
    public:
        virtual ~IJpegEncoder() = default;

        virtual unsigned int encodeImage(cv::Mat inputImage,
                                         unsigned char *outputBuffer,
                                         unsigned int maxOutputLength,
                                         int quality) = 0;

        virtual unsigned int encodeImage(cv::Mat inputImage,
                                         unsigned char *outputBuffer,
                                         unsigned int maxOutputLength) {
            return encodeImage(inputImage, outputBuffer, maxOutputLength, DEFAULT_JPEG_QUALITY);
        }
    };

    class OpenCvJpegEncoder : public IJpegEncoder, public wallaroo::Part {
    public:

        unsigned int encodeImage(cv::Mat inputImage,
                                 unsigned char *outputBuffer,
                                 unsigned int maxOutputLength,
                                 int quality) override;

    private:
        log4cpp::Category &logger = log4cpp::Category::getInstance("OpenCvJpegEncoder");
    };

    class TurboJpegEncoder : public IJpegEncoder, public wallaroo::Part {
    public:
        TurboJpegEncoder();

        ~TurboJpegEncoder();

        unsigned int encodeImage(cv::Mat inputImage,
                                 unsigned char *outputBuffer,
                                 unsigned int maxOutputLength,
                                 int quality) override;

    private:
        log4cpp::Category &logger = log4cpp::Category::getInstance("TurboJpegEncoder");

        tjhandle _jpegCompressor;
    };
}