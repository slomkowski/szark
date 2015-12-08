#include "JpegEncoder.hpp"

#include <wallaroo/class.h>

using namespace camera;

const int JPEG_QUALITY = 45;

WALLAROO_REGISTER(OpenCvJpegEncoder);

unsigned int camera::OpenCvJpegEncoder::encodeImage(cv::Mat inputImage,
                                                    unsigned char *outputBuffer,
                                                    unsigned int maxOutputLength) {

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
