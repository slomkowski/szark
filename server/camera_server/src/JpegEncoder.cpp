#include "JpegEncoder.hpp"

#include "utils.hpp"

#include <log4cpp/Category.hh>
#include <wallaroo/part.h>
#include <wallaroo/registered.h>

#include <turbojpeg.h>

#include <vector>
#include <cstring>

using namespace camera;

class OpenCvJpegEncoder : public IJpegEncoder, public wallaroo::Part {
public:

    unsigned int encodeImage(cv::Mat inputImage,
                             unsigned char *outputBuffer,
                             unsigned int maxOutputLength,
                             int quality) override {
        std::vector<unsigned char> buffer(30000);

        std::vector<int> jpegEncoderParameters;
        jpegEncoderParameters.push_back(CV_IMWRITE_JPEG_QUALITY);
        jpegEncoderParameters.push_back(quality);

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

private:
    log4cpp::Category &logger = log4cpp::Category::getInstance("OpenCvJpegEncoder");
};

class TurboJpegEncoder : public IJpegEncoder, public wallaroo::Part {
public:
    TurboJpegEncoder() {
        _jpegCompressor = tjInitCompress();
    }

    virtual ~TurboJpegEncoder() {
        tjDestroy(_jpegCompressor);
    }

    unsigned int encodeImage(cv::Mat inputImage,
                             unsigned char *outputBuffer,
                             unsigned int maxOutputLength,
                             int quality) override {
        long unsigned int _jpegSize = maxOutputLength;

        int us = common::utils::measureTime<std::chrono::microseconds>([&]() {

            int status = tjCompress2(_jpegCompressor, inputImage.data, inputImage.cols, 0, inputImage.rows, TJPF_BGR,
                                     &outputBuffer, &_jpegSize, TJSAMP_422, quality,
                                     TJFLAG_FASTDCT | TJFLAG_NOREALLOC);

            if (status != 0) {
                throw std::runtime_error((boost::format("tjCompress2 error: %s") % tjGetErrorStr()).str());
            }
        });

        logger.info((boost::format("Converted image to JPEG in %u us.") % us).str());

        return _jpegSize;
    }

private:
    log4cpp::Category &logger = log4cpp::Category::getInstance("TurboJpegEncoder");

    tjhandle _jpegCompressor;
};

WALLAROO_REGISTER(OpenCvJpegEncoder);

WALLAROO_REGISTER(TurboJpegEncoder);
