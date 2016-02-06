#include "JpegEncoder.hpp"

#include <wallaroo/registered.h>


using namespace camera;

const int JPEG_QUALITY = 45;

WALLAROO_REGISTER(OpenCvJpegEncoder);

WALLAROO_REGISTER(TurboJpegEncoder);

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

    logger.info("Converted image to JPEG in %u us.", us);

    if (buffer.size() > maxOutputLength) {
        throw std::runtime_error("encoded JPEG greater than the available buffer");
    }

    std::copy(buffer.begin(), buffer.end(), outputBuffer);

    return buffer.size();
}

unsigned int TurboJpegEncoder::encodeImage(cv::Mat inputImage,
                                           unsigned char *outputBuffer,
                                           unsigned int maxOutputLength) {
    long unsigned int _jpegSize = maxOutputLength;

    int us = common::utils::measureTime<std::chrono::microseconds>([&]() {

        int status = tjCompress2(_jpegCompressor, inputImage.data, inputImage.cols, 0, inputImage.rows, TJPF_BGR,
                                 &outputBuffer, &_jpegSize, TJSAMP_422, JPEG_QUALITY,
                                 TJFLAG_FASTDCT | TJFLAG_NOREALLOC);

        if (status != 0) {
            throw std::runtime_error((boost::format("tjCompress2 error: %s") % tjGetErrorStr()).str());
        }
    });

    logger.info((boost::format("Converted image to JPEG in %u us.") % us).str());

    return _jpegSize;
}

TurboJpegEncoder::TurboJpegEncoder() {
    _jpegCompressor = tjInitCompress();

}

TurboJpegEncoder::~TurboJpegEncoder() {
    tjDestroy(_jpegCompressor);
}
