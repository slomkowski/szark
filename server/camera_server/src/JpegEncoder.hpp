#pragma once

#include <opencv2/opencv.hpp>

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>

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
}