#pragma once

#include "utils.hpp"

#include <boost/format.hpp>

#include <vector>
#include <cstring>

namespace camera {
    class IJpegEncoder : boost::noncopyable {
    public:
        virtual ~IJpegEncoder() = default;

        virtual unsigned int encodeImage(cv::Mat inputImage, unsigned char *outputBuffer,
                                         unsigned int maxOutputLength) = 0;
    };

    const int JPEG_QUALITY = 45;

    class OpenCvJpegEncoder : public IJpegEncoder {
    public:

        unsigned int encodeImage(cv::Mat inputImage, unsigned char *outputBuffer, unsigned int maxOutputLength) {

            std::vector<unsigned char> buffer(30000);

            std::vector<int> jpegEncoderParameters;
            jpegEncoderParameters.push_back(CV_IMWRITE_JPEG_QUALITY);
            jpegEncoderParameters.push_back(JPEG_QUALITY);

            int us = common::utils::measureTime<std::chrono::microseconds>([&]() {
                cv::imencode(".jpg", inputImage, buffer, jpegEncoderParameters);
            });

            logger.info((boost::format("Converted image to JPEG in %u us.") % us).str());

            if (buffer.size() > maxOutputLength) {
                throw std::runtime_error("encoded JPEG greater than the available buffer");
            }

            std::copy(buffer.begin(), buffer.end(), outputBuffer);

            return buffer.size();
        }

    private:
        log4cpp::Category &logger = log4cpp::Category::getInstance("OpenCvJpegEncoder");

    };
}